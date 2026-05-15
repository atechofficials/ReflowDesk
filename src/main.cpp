/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * main.cpp: Main application logic and loop
 */

#include <Arduino.h>
#include <Wire.h>

#include "alerts.h"
#include "fan.h"
#include "heater.h"
#include "input.h"
#include "reflow.h"
#include "sensors.h"
#include "settings.h"
#include "ui.h"
#include "web_server.h"

TwoWire AppWire(0);

SettingsStore settings(AppWire);
SensorManager sensors(AppWire);
HeaterController heater;
#if REFLOW_HAS_BOARD_FAN
FanController fan(Pins::FAN_PWM, Pins::FAN_TACH, Pins::FAN_POWER, 0);
FanController boardFan(Pins::FAN_PWM2, Pins::FAN_TACH2, FanController::NO_POWER_PIN, 1);
#else
FanController fan;
#endif
AlertManager alerts;
InputManager input;
ReflowController reflow(heater, fan, alerts);
UiManager ui(AppWire);
ReflowWebServer webServer;

#if REFLOW_HAS_BOARD_FAN
uint8_t boardFanDutyFor(float boardC) {
  if (boardC >= BoardCooling::FAN_FULL_C) {
    return BoardCooling::FAN_MAX_PERCENT;
  }
  if (boardC <= BoardCooling::FAN_ON_C) {
    return BoardCooling::FAN_MIN_PERCENT;
  }

  float ratio = (boardC - BoardCooling::FAN_ON_C) / (BoardCooling::FAN_FULL_C - BoardCooling::FAN_ON_C);
  float duty = BoardCooling::FAN_MIN_PERCENT +
               ratio * (BoardCooling::FAN_MAX_PERCENT - BoardCooling::FAN_MIN_PERCENT);
  return static_cast<uint8_t>(duty + 0.5f);
}

void updateBoardFan(uint32_t now, const TemperatureSample &sample) {
  static bool boardFanActive = false;
  boardFan.update(now);

#if REFLOW_HAS_BOARD_NTC
  if (sample.boardReadOnce && !sample.boardOk) {
    boardFanActive = true;
    boardFan.setSpeed(BoardCooling::FAN_NTC_FAIL_PERCENT);
    return;
  }

  if (!sample.boardReadOnce) {
    boardFanActive = false;
    boardFan.stop();
    return;
  }

  if (boardFanActive) {
    if (sample.boardC <= BoardCooling::FAN_OFF_C) {
      boardFanActive = false;
      boardFan.stop();
      return;
    }
  } else if (sample.boardC < BoardCooling::FAN_ON_C) {
    boardFan.stop();
    return;
  }

  boardFanActive = true;
  boardFan.setSpeed(boardFanDutyFor(sample.boardC));
#else
  boardFanActive = false;
  boardFan.stop();
#endif
}
#endif

#if REFLOW_DEBUG
void printSettingsLine(const SettingsData &data) {
  const ReflowProfile &profile = SettingsStore::activeProfile(data);
  Serial.print(F("CFG ph="));
  Serial.print(profile.preheatTempC);
  Serial.print(F("/"));
  Serial.print(profile.preheatSeconds);
  Serial.print(F(" sk="));
  Serial.print(profile.soakTempC);
  Serial.print(F("/"));
  Serial.print(profile.soakSeconds);
  Serial.print(F(" rf="));
  Serial.print(profile.reflowTempC);
  Serial.print(F("/"));
  Serial.print(profile.reflowSeconds);
  Serial.print(F(" profile="));
  Serial.print(data.selectedProfileIndex + 1);
  Serial.print(F(":"));
  Serial.print(profile.name);
  Serial.print(F(" safe="));
  Serial.print(data.safeTouchC);
  Serial.print(F(" cut="));
  Serial.print(data.safetyCutoffC);
  Serial.print(F(" buzLvl="));
  Serial.print(data.buzzerLevel);
  Serial.print(F(" led="));
  Serial.print(data.ledBrightness);
  Serial.print(F(" oled="));
  Serial.print(data.oledBrightness);
  Serial.print(F(" sleep="));
  Serial.print(data.oledSleepTimeoutSeconds);
  Serial.print(F("s lock="));
  Serial.print(data.deviceControlsLocked ? F("on") : F("off"));
  Serial.print(F(" oledOff="));
  Serial.print(data.oledForcedOff ? F("on") : F("off"));
  Serial.print(F(" cool="));
  Serial.print(SettingsStore::coolingProfileName(profile.coolingProfile));
  Serial.print(F(" pid="));
  Serial.print(SettingsStore::kp(data), 2);
  Serial.print(F(","));
  Serial.print(SettingsStore::ki(data), 2);
  Serial.print(F(","));
  Serial.println(SettingsStore::kd(data), 2);
}

void printStageLine(uint32_t now, const SettingsData &data) {
  Serial.print(F("EV t="));
  Serial.print(now);
  Serial.print(F(" stage="));
  Serial.print(reflow.stateName());
  Serial.print(F(" set="));
  Serial.print(reflow.targetC(data), 0);
  Serial.print(F(" hold="));
  Serial.print(reflow.holdTargetSeconds(data));
  Serial.println();
}

const __FlashStringHelper *stateNameFor(ReflowState state) {
  switch (state) {
    case ReflowState::Idle:
      return F("Idle");
    case ReflowState::Preheat:
      return F("Preheat");
    case ReflowState::Soak:
      return F("Soak");
    case ReflowState::Reflow:
      return F("Reflow");
    case ReflowState::Cooling:
      return F("Cooling");
    case ReflowState::Fault:
      return F("Fault");
    case ReflowState::Aborted:
      return F("Aborted");
  }
  return F("?");
}

void printEventLog(uint32_t now) {
  static bool initialized = false;
  static ReflowState lastState = ReflowState::Idle;
  static FaultReason lastFault = FaultReason::None;

  ReflowState state = reflow.state();
  FaultReason fault = reflow.faultReason();
  if (!initialized) {
    initialized = true;
    lastState = state;
    lastFault = fault;
    return;
  }

  if (state == lastState && fault == lastFault) {
    return;
  }

  const SettingsData &data = settings.data();
  if (state != lastState) {
    if (state == ReflowState::Preheat) {
      Serial.print(F("EV start t="));
      Serial.println(now);
      printSettingsLine(data);
      printStageLine(now, data);
    } else if (state == ReflowState::Soak || state == ReflowState::Reflow) {
      printStageLine(now, data);
    } else if (state == ReflowState::Cooling) {
      Serial.println(F("EV cooling start reason=profile-complete"));
    } else if (state == ReflowState::Aborted) {
      Serial.println(F("EV abort state=entered h=off fan=100"));
    } else if (state == ReflowState::Idle && lastState == ReflowState::Cooling) {
      Serial.println(F("EV cooling done safe=ready"));
    } else if (state == ReflowState::Idle && lastState == ReflowState::Aborted) {
      Serial.println(F("EV abort cooldown done safe=ready"));
    } else if (state == ReflowState::Idle && (lastState == ReflowState::Preheat || lastState == ReflowState::Soak ||
                                             lastState == ReflowState::Reflow)) {
      Serial.print(F("EV stop from="));
      Serial.print(stateNameFor(lastState));
      Serial.println(F(" reason=instant-safe-stop"));
    }
  }

  if (state == ReflowState::Fault && fault != lastFault) {
    Serial.print(F("EV fault="));
    Serial.print(reflow.faultName());
    Serial.println(F(" h=off fan=100"));
  }

  lastState = state;
  lastFault = fault;
}

void printDebugLine(uint32_t now) {
  const TemperatureSample &sample = sensors.sample();
  Serial.print(F("t="));
  Serial.print(now);
  Serial.print(F(" state="));
  Serial.print(reflow.stateName());
  Serial.print(F(" set="));
  Serial.print(reflow.targetC(settings.data()), 0);
  Serial.print(F(" plate="));
  if (sample.plateOk) {
    Serial.print(sample.plateC, 1);
  } else {
    Serial.print(F("ERR"));
  }
  Serial.print(F(" amb="));
  if (sample.ambientOk) {
    Serial.print(sample.ambientC, 1);
  } else {
    Serial.print(F("ERR"));
  }
#if REFLOW_HAS_BOARD_NTC
  Serial.print(F(" board="));
  if (sample.boardOk) {
    Serial.print(sample.boardC, 1);
  } else {
    Serial.print(F("ERR"));
  }
  Serial.print(F(" boardAdc="));
  Serial.print(sample.boardAdc);
#endif
  Serial.print(F(" raw=0x"));
  Serial.print(sample.plateRaw, HEX);
  Serial.print(F(" status=0x"));
  Serial.print(sample.plateStatus, HEX);
  Serial.print(F(" duty="));
  Serial.print(heater.dutyPercent(), 0);
  Serial.print(F(" h="));
  Serial.print(heater.outputOn() ? F("ON") : F("OFF"));
  Serial.print(F(" pin="));
  Serial.print(heater.ssrPinHigh() ? F("H") : F("L"));
  Serial.print(F(" fan="));
  Serial.print(fan.speedPercent());
  Serial.print(F(" fp="));
  Serial.print(fan.powerEnabled() ? F("ON") : F("OFF"));
  Serial.print(F(" rpm="));
  Serial.print(fan.rpm());
  if (fan.failed()) {
    Serial.print(F(" fanFail=1"));
  }
#if REFLOW_HAS_BOARD_FAN
  Serial.print(F(" fan2="));
  Serial.print(boardFan.speedPercent());
  Serial.print(F(" rpm2="));
  Serial.print(boardFan.rpm());
  if (boardFan.failed()) {
    Serial.print(F(" fan2Fail=1"));
  }
#endif
  Serial.println();
}
#endif

void setup() {
  heater.begin();

#if REFLOW_DEBUG
  Serial.begin(115200);
  uint32_t serialStart = millis();
  while (!Serial && millis() - serialStart < 1500) {
    delay(10);
  }
#endif

  fan.begin();
#if REFLOW_HAS_BOARD_FAN
  boardFan.begin();
#endif
  alerts.begin();

  AppWire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
  AppWire.setClock(400000);
  settings.begin();
  settings.validate();
  alerts.setBuzzerLevel(settings.data().buzzerLevel);
  alerts.setLedBrightness(settings.data().ledBrightness);

  sensors.begin();
  input.begin();
  reflow.begin();
  ui.begin();

#if REFLOW_WEB_ENABLED
#if REFLOW_HAS_BOARD_FAN
  webServer.begin(settings, sensors, reflow, heater, fan, alerts, ui, &boardFan);
#else
  webServer.begin(settings, sensors, reflow, heater, fan, alerts, ui);
#endif
#endif

  alerts.beepPattern(2, 70, 80);

#if REFLOW_DEBUG
  Serial.println(F("SMD Reflow Plate boot"));
  printDebugLine(millis());
#endif
}

void loop() {
  uint32_t now = millis();
  sensors.update(now);

  const TemperatureSample &sample = sensors.sample();
  bool controlsLocked = settings.data().deviceControlsLocked != 0;
#if REFLOW_DEBUG
  static bool controlsLockInitialized = false;
  static bool lastControlsLocked = false;
  static bool lastOledForcedOff = false;
  bool oledForcedOff = settings.data().oledForcedOff != 0;
  if (!controlsLockInitialized) {
    controlsLockInitialized = true;
    lastControlsLocked = controlsLocked;
    lastOledForcedOff = oledForcedOff;
  } else if (controlsLocked != lastControlsLocked || oledForcedOff != lastOledForcedOff) {
    lastControlsLocked = controlsLocked;
    lastOledForcedOff = oledForcedOff;
    Serial.print(F("EV controls lock="));
    Serial.print(controlsLocked ? F("on") : F("off"));
    Serial.print(F(" oledOff="));
    Serial.println(oledForcedOff ? F("on") : F("off"));
  }
#endif
  bool rotationLocked = controlsLocked || reflow.isHeatingState() || reflow.state() == ReflowState::Cooling ||
                        reflow.cooldownLocked(settings.data(), sample, now) || reflow.isFaultLike();
  if (rotationLocked) {
    input.discardRotation();
  }

  InputEvent event = input.poll(now);
  if (controlsLocked) {
    event = {};
  }
  ui.handleInput(event, settings, reflow, alerts, sample);
  ui.syncSettingsRevision(settings);

  reflow.update(now, settings.data(), sample);
#if REFLOW_HAS_BOARD_FAN
  updateBoardFan(now, sample);
#endif
  if (!reflow.isHeatingState()) {
    heater.forceOff();
  }
#if REFLOW_DEBUG
  printEventLog(now);
#endif
  alerts.setBuzzerLevel(settings.data().buzzerLevel);
  alerts.updateStatusLed(sample.plateC, settings.data(), reflow.isFaultLike());
  alerts.tick(now);
#if REFLOW_HAS_BOARD_FAN
  ui.draw(now, settings, reflow, heater, fan, sample, &boardFan);
#else
  ui.draw(now, settings, reflow, heater, fan, sample);
#endif
#if REFLOW_WEB_ENABLED
  webServer.loop(now);
#endif
  if (!reflow.isHeatingState()) {
    heater.forceOff();
  }

#if REFLOW_DEBUG
  static uint32_t lastDebugMs = 0;
  if (now - lastDebugMs >= 1000) {
    lastDebugMs = now;
    printDebugLine(now);
  }
#endif
}

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

TwoWire AppWire(0);

SettingsStore settings(AppWire);
SensorManager sensors(AppWire);
HeaterController heater;
FanController fan;
AlertManager alerts;
InputManager input;
ReflowController reflow(heater, fan, alerts);
UiManager ui(AppWire);

#if REFLOW_DEBUG
void printSettingsLine(const SettingsData &data) {
  Serial.print(F("CFG ph="));
  Serial.print(data.preheatTempC);
  Serial.print(F("/"));
  Serial.print(data.preheatSeconds);
  Serial.print(F(" sk="));
  Serial.print(data.soakTempC);
  Serial.print(F("/"));
  Serial.print(data.soakSeconds);
  Serial.print(F(" rf="));
  Serial.print(data.reflowTempC);
  Serial.print(F("/"));
  Serial.print(data.reflowSeconds);
  Serial.print(F(" safe="));
  Serial.print(data.safeTouchC);
  Serial.print(F(" cut="));
  Serial.print(data.safetyCutoffC);
  Serial.print(F(" buz="));
  Serial.print(data.buzzerEnabled ? F("on") : F("off"));
  Serial.print(F(" cool="));
  Serial.print(SettingsStore::coolingProfileName(data.coolingProfile));
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
      Serial.println(F("EV finish cool"));
    } else if (state == ReflowState::Aborted) {
      Serial.println(F("EV abort h=off fan=100"));
    } else if (state == ReflowState::Idle && lastState == ReflowState::Cooling) {
      Serial.println(F("EV idle safe"));
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
  alerts.begin();

  AppWire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
  AppWire.setClock(400000);
  settings.begin();
  settings.validate();
  alerts.setBuzzerEnabled(settings.data().buzzerEnabled != 0);
  alerts.setLedBrightness(settings.data().ledBrightness);

  sensors.begin();
  input.begin();
  reflow.begin();
  ui.begin();

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
  bool rotationLocked = reflow.isHeatingState() || reflow.state() == ReflowState::Cooling ||
                        reflow.cooldownLocked(settings.data(), sample, now) || reflow.isFaultLike();
  if (rotationLocked) {
    input.discardRotation();
  }

  InputEvent event = input.poll(now);
  ui.handleInput(event, settings, reflow, alerts, sample);

  reflow.update(now, settings.data(), sample);
  if (!reflow.isHeatingState()) {
    heater.forceOff();
  }
#if REFLOW_DEBUG
  printEventLog(now);
#endif
  alerts.setBuzzerEnabled(settings.data().buzzerEnabled != 0);
  alerts.updateStatusLed(sample.plateC, settings.data(), reflow.isFaultLike());
  alerts.tick(now);
  ui.draw(now, settings, reflow, heater, fan, sample);
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

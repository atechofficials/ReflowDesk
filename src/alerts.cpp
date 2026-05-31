/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * alerts.cpp: Buzzer and status LED alert handling
 */

#include "alerts.h"

namespace {
constexpr uint32_t BUZZER_PWM_FREQ_HZ = 2700;
constexpr uint8_t BUZZER_PWM_BITS = 8;
constexpr uint32_t STATUS_LED_WIFI_BLINK_MS = 500;
#if !(defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3)
constexpr uint8_t BUZZER_PWM_CHANNEL = 6;
#endif

uint8_t buzzerDutyForLevel(uint8_t level) {
  static constexpr uint8_t dutyByLevel[] = {0, 32, 64, 96, 144, 220};
  if (level > 5) {
    level = 5;
  }
  return dutyByLevel[level];
}
}

void AlertManager::begin() {
  pinMode(Pins::BUZZER, OUTPUT);
  attachBuzzerPwm();
  stopBuzzer();
  _led.begin();
  _led.setBrightness(45);
  _led.show();
}

void AlertManager::setLedBrightness(uint8_t brightness) {
  _led.setBrightness(brightness);
  _led.show();
}

void AlertManager::setBuzzerLevel(uint8_t level) {
  _buzzerLevel = level > 5 ? 5 : level;
  if (_buzzerLevel == 0) {
    stopBuzzer();
  }
}

void AlertManager::beep(uint16_t durationMs) {
  beepPattern(1, durationMs, 0);
}

void AlertManager::beepPattern(uint8_t count, uint16_t durationMs, uint16_t gapMs) {
  if (_buzzerLevel == 0 || count == 0 || durationMs == 0) {
    return;
  }
  _remainingBeeps = count;
  _beepMs = durationMs;
  _gapMs = gapMs;
  _buzzing = false;
  _nextToggleMs = 0;
}

void AlertManager::tick(uint32_t now) {
  if (_buzzerLevel == 0) {
    stopBuzzer();
    return;
  }
  if (_remainingBeeps == 0) {
    return;
  }
  if (_nextToggleMs != 0 && static_cast<int32_t>(now - _nextToggleMs) < 0) {
    return;
  }

  if (!_buzzing) {
    writeBuzzer(true);
    _buzzing = true;
    _nextToggleMs = now + _beepMs;
  } else {
    writeBuzzer(false);
    _buzzing = false;
    --_remainingBeeps;
    _nextToggleMs = _remainingBeeps > 0 ? now + _gapMs : 0;
  }
}

void AlertManager::blinkStatusWhite(uint32_t now) {
  const bool ledOn = ((now / STATUS_LED_WIFI_BLINK_MS) % 2U) == 0U;
  setLed(ledOn ? 255 : 0, ledOn ? 255 : 0, ledOn ? 255 : 0);
}

void AlertManager::clearStatusLed() {
  setLed(0, 0, 0);
}

void AlertManager::updateStatusLed(float plateC, const SettingsData &settings, bool fault) {
  const ReflowProfile &profile = SettingsStore::activeProfile(settings);
  if (fault) {
    setLed(255, 0, 0);
  } else if (plateC <= settings.safeTouchC) {
    setLed(0, 180, 0);
  } else if (plateC < profile.soakTempC) {
    setLed(220, 170, 0);
  } else if (plateC < profile.reflowTempC) {
    setLed(255, 70, 0);
  } else {
    setLed(255, 0, 0);
  }
}

void AlertManager::setLed(uint8_t r, uint8_t g, uint8_t b) {
  _led.setPixelColor(0, _led.Color(r, g, b));
  _led.show();
}

void AlertManager::attachBuzzerPwm() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(Pins::BUZZER, BUZZER_PWM_FREQ_HZ, BUZZER_PWM_BITS);
#else
  ledcSetup(BUZZER_PWM_CHANNEL, BUZZER_PWM_FREQ_HZ, BUZZER_PWM_BITS);
  ledcAttachPin(Pins::BUZZER, BUZZER_PWM_CHANNEL);
#endif
}

void AlertManager::writeBuzzer(bool enabled) {
  const uint8_t duty = enabled ? buzzerDutyForLevel(_buzzerLevel) : 0;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(Pins::BUZZER, duty);
#else
  ledcWrite(BUZZER_PWM_CHANNEL, duty);
#endif
}

void AlertManager::stopBuzzer() {
  _remainingBeeps = 0;
  _buzzing = false;
  _nextToggleMs = 0;
  writeBuzzer(false);
}

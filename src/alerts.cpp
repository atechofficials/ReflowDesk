/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * alerts.cpp: Buzzer and status LED alert handling
 */

#include "alerts.h"

void AlertManager::begin() {
  pinMode(Pins::BUZZER, OUTPUT);
  digitalWrite(Pins::BUZZER, LOW);
  _led.begin();
  _led.setBrightness(45);
  _led.show();
}

void AlertManager::setLedBrightness(uint8_t brightness) {
  _led.setBrightness(brightness);
  _led.show();
}

void AlertManager::setBuzzerEnabled(bool enabled) {
  _buzzerEnabled = enabled;
  if (!_buzzerEnabled) {
    _remainingBeeps = 0;
    _buzzing = false;
    _nextToggleMs = 0;
    digitalWrite(Pins::BUZZER, LOW);
  }
}

void AlertManager::beep(uint16_t durationMs) {
  beepPattern(1, durationMs, 0);
}

void AlertManager::beepPattern(uint8_t count, uint16_t durationMs, uint16_t gapMs) {
  if (!_buzzerEnabled || count == 0 || durationMs == 0) {
    return;
  }
  _remainingBeeps = count;
  _beepMs = durationMs;
  _gapMs = gapMs;
  _buzzing = false;
  _nextToggleMs = 0;
}

void AlertManager::tick(uint32_t now) {
  if (!_buzzerEnabled) {
    _remainingBeeps = 0;
    _buzzing = false;
    _nextToggleMs = 0;
    digitalWrite(Pins::BUZZER, LOW);
    return;
  }
  if (_remainingBeeps == 0) {
    return;
  }
  if (_nextToggleMs != 0 && static_cast<int32_t>(now - _nextToggleMs) < 0) {
    return;
  }

  if (!_buzzing) {
    digitalWrite(Pins::BUZZER, HIGH);
    _buzzing = true;
    _nextToggleMs = now + _beepMs;
  } else {
    digitalWrite(Pins::BUZZER, LOW);
    _buzzing = false;
    --_remainingBeeps;
    _nextToggleMs = _remainingBeeps > 0 ? now + _gapMs : 0;
  }
}

void AlertManager::updateStatusLed(float plateC, const SettingsData &settings, bool fault) {
  if (fault) {
    setLed(255, 0, 0);
  } else if (plateC <= settings.safeTouchC) {
    setLed(0, 180, 0);
  } else if (plateC < settings.soakTempC) {
    setLed(220, 170, 0);
  } else if (plateC < settings.reflowTempC) {
    setLed(255, 70, 0);
  } else {
    setLed(255, 0, 0);
  }
}

void AlertManager::setLed(uint8_t r, uint8_t g, uint8_t b) {
  _led.setPixelColor(0, _led.Color(r, g, b));
  _led.show();
}

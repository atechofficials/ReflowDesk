/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * fan.cpp: Cooling fan PWM, power, and tachometer control
 */

#include "fan.h"

namespace {
constexpr uint32_t FAN_PWM_FREQ_HZ = 25000;
constexpr uint8_t FAN_PWM_BITS = 8;
constexpr uint8_t FAN_PWM_TOP = 255;
constexpr uint8_t FAN_MIN_RUNNING_PERCENT = 25;
constexpr uint32_t FAN_STARTUP_ASSIST_MS = 5000;
constexpr uint32_t FAN_RPM_SAMPLE_MS = 1000;
constexpr uint32_t FAN_STALL_GRACE_MS = 3000;
constexpr uint16_t FAN_MIN_OK_RPM = 300;
constexpr uint8_t FAN_TACH_PULSES_PER_REV = 2;
constexpr uint32_t FAN_TACH_MIN_EDGE_US = 2000;
constexpr uint8_t FAN_POWER_ON_LEVEL = HIGH;
constexpr uint8_t FAN_POWER_OFF_LEVEL = LOW;
}

FanController::FanController(uint32_t pwmPin, uint32_t tachPin, uint32_t powerPin, uint8_t pwmChannel)
    : _pwmPin(pwmPin), _tachPin(tachPin), _powerPin(powerPin), _pwmChannel(pwmChannel) {}

void FanController::begin() {
  pinMode(_pwmPin, OUTPUT);
  if (_powerPin != NO_POWER_PIN) {
    pinMode(_powerPin, OUTPUT);
  }
  pinMode(_tachPin, INPUT);
  attachFanPwm();
  stop();
  resetTachWindow(millis());
  attachInterruptArg(digitalPinToInterrupt(_tachPin), FanController::tachIsr, this, FALLING);
}

void FanController::update(uint32_t now) {
  if (_lastRpmMs == 0) {
    resetTachWindow(now);
  }

  if (now - _lastRpmMs >= FAN_RPM_SAMPLE_MS) {
    uint32_t elapsedMs = now - _lastRpmMs;
    noInterrupts();
    uint32_t pulses = _tachPulses;
    _tachPulses = 0;
    _tachRejected = 0;
    interrupts();

    if (elapsedMs > 0) {
      uint32_t rpm = (pulses * 60000UL) / (elapsedMs * FAN_TACH_PULSES_PER_REV);
      _rpm = static_cast<uint16_t>(rpm > 65535UL ? 65535UL : rpm);
    }
    _lastRpmMs = now;
  }

  bool shouldRun = _powerEnabled && _appliedPercent > 0;
  if (!shouldRun) {
    _stallStartMs = 0;
    return;
  }

  if (static_cast<int32_t>(now - _startupAssistUntilMs) < 0) {
    _stallStartMs = 0;
    return;
  }

  if (_rpm >= FAN_MIN_OK_RPM) {
    _stallStartMs = 0;
    return;
  }

  if (_stallStartMs == 0) {
    _stallStartMs = now;
  } else if (now - _stallStartMs >= FAN_STALL_GRACE_MS) {
    _failed = true;
  }
}

void FanController::setSpeed(uint8_t percent) {
  if (percent > 100) {
    percent = 100;
  }

  _requestedPercent = percent;
  if (percent == 0) {
    stop();
    return;
  }

  uint32_t now = millis();
  if (!_powerEnabled) {
    _startupAssistUntilMs = now + FAN_STARTUP_ASSIST_MS;
    _stallStartMs = 0;
    _failed = false;
    resetTachWindow(now);
    writeFanPwm(FAN_MIN_RUNNING_PERCENT);
    writeFanPower(true);
    return;
  }

  if (static_cast<int32_t>(now - _startupAssistUntilMs) < 0) {
    writeFanPwm(FAN_MIN_RUNNING_PERCENT);
  } else {
    writeFanPwm(constrainedDuty(percent));
  }
}

void FanController::stop() {
  _requestedPercent = 0;
  _startupAssistUntilMs = 0;
  _stallStartMs = 0;
  _failed = false;
  _rpm = 0;
  writeFanPwm(0);
  writeFanPower(false);
  resetTachWindow(millis());
}

void FanController::tachIsr(void *arg) {
  FanController *fan = static_cast<FanController *>(arg);
  if (fan == nullptr) {
    return;
  }

  uint32_t nowUs = micros();
  uint32_t deltaUs = nowUs - fan->_lastTachUs;
  if (deltaUs >= FAN_TACH_MIN_EDGE_US) {
    fan->_tachPulses = fan->_tachPulses + 1;
    fan->_lastTachUs = nowUs;
  } else {
    fan->_tachRejected = fan->_tachRejected + 1;
  }
}

bool FanController::attachFanPwm() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  return ledcAttach(_pwmPin, FAN_PWM_FREQ_HZ, FAN_PWM_BITS);
#else
  ledcSetup(_pwmChannel, FAN_PWM_FREQ_HZ, FAN_PWM_BITS);
  ledcAttachPin(_pwmPin, _pwmChannel);
  return true;
#endif
}

uint8_t FanController::constrainedDuty(uint8_t percent) const {
  if (percent == 0) {
    return 0;
  }
  return percent < FAN_MIN_RUNNING_PERCENT ? FAN_MIN_RUNNING_PERCENT : percent;
}

void FanController::writeFanPower(bool enabled) {
  _powerEnabled = enabled;
  if (_powerPin != NO_POWER_PIN) {
    digitalWrite(_powerPin, enabled ? FAN_POWER_ON_LEVEL : FAN_POWER_OFF_LEVEL);
  }
}

void FanController::writeFanPwm(uint8_t fanHighPercent) {
  if (fanHighPercent > 100) {
    fanHighPercent = 100;
  }
  _appliedPercent = fanHighPercent;

  uint16_t fanHighCounts = (static_cast<uint16_t>(fanHighPercent) * FAN_PWM_TOP + 50) / 100;
  uint8_t invertedDriveCounts = FAN_PWM_TOP - static_cast<uint8_t>(fanHighCounts);
  writeFanPwmCounts(invertedDriveCounts);
}

void FanController::writeFanPwmCounts(uint8_t counts) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(_pwmPin, counts);
#else
  ledcWrite(_pwmChannel, counts);
#endif
}

void FanController::resetTachWindow(uint32_t now) {
  noInterrupts();
  _tachPulses = 0;
  _tachRejected = 0;
  _lastTachUs = micros();
  interrupts();
  _lastRpmMs = now;
}

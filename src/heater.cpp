/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * heater.cpp: Heater output and PID time-window control
 */

#include "heater.h"

namespace {
constexpr uint8_t SSR_DRIVER_ON_LEVEL = HIGH;
constexpr uint8_t SSR_DRIVER_OFF_LEVEL = LOW;

void configureSsrOff() {
  pinMode(Pins::HEATER_CTRL, OUTPUT);
  digitalWrite(Pins::HEATER_CTRL, SSR_DRIVER_OFF_LEVEL);
}

void configureSsrOn() {
  pinMode(Pins::HEATER_CTRL, OUTPUT);
  digitalWrite(Pins::HEATER_CTRL, SSR_DRIVER_ON_LEVEL);
}

void writeSsrElectrical(bool on) {
  if (on) {
    configureSsrOn();
  } else {
    configureSsrOff();
  }
}

bool readSsrPinHigh() {
  return digitalRead(Pins::HEATER_CTRL) == SSR_DRIVER_OFF_LEVEL;
}

float clampFloat(float value, float low, float high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}
}

void HeaterController::begin() {
  writeSsrElectrical(false);
  _enabled = false;
  _outputOn = false;
  _dutyPercent = 0.0f;
  _windowStartMs = millis();
}

void HeaterController::configure(float kp, float ki, float kd) {
  _kp = kp;
  _ki = ki;
  _kd = kd;
}

void HeaterController::reset() {
  _integral = 0.0f;
  _hasLastInput = false;
  _lastPidMs = 0;
  _dutyPercent = 0.0f;
  _windowStartMs = millis();
  writeOutput(false);
}

void HeaterController::setEnabled(bool enabled) {
  _enabled = enabled;
  if (!enabled) {
    _dutyPercent = 0.0f;
    writeOutput(false);
  }
}

void HeaterController::forceOff() {
  _enabled = false;
  _dutyPercent = 0.0f;
  _outputOn = false;
  writeSsrElectrical(false);
}

void HeaterController::updatePid(uint32_t now, float currentC, float targetC, float ambientC) {
  if (!_enabled) {
    return;
  }

  float dt = _lastPidMs == 0 ? (Timing::CONTROL_MS / 1000.0f) : (now - _lastPidMs) / 1000.0f;
  if (dt <= 0.0f) {
    dt = Timing::CONTROL_MS / 1000.0f;
  }
  _lastPidMs = now;

  float error = targetC - currentC;
  _integral = clampFloat(_integral + (error * dt), -600.0f, 600.0f);
  float derivative = 0.0f;
  if (_hasLastInput) {
    derivative = (currentC - _lastInput) / dt;
  }
  _lastInput = currentC;
  _hasLastInput = true;

  float ambientBoost = clampFloat((targetC - ambientC) * 0.08f, 0.0f, 8.0f);
  float output = (_kp * error) + (_ki * _integral) - (_kd * derivative) + ambientBoost;

  // Keep the SSR from backing off during an intentional ramp. The PID still
  // controls the final approach and hold zone, but a large remaining error
  // means the plate needs full heat rather than derivative-limited pulses.
  if (error >= 12.0f) {
    output = 100.0f;
  } else if (error >= 6.0f && output < 70.0f) {
    output = 70.0f;
  }

  if (currentC >= targetC + 4.0f) {
    output = 0.0f;
  }
  _dutyPercent = clampFloat(output, 0.0f, 100.0f);
}

void HeaterController::serviceWindow(uint32_t now) {
  if (!_enabled || _dutyPercent <= 0.0f) {
    writeOutput(false);
    return;
  }
  if (now - _windowStartMs >= Timing::SSR_WINDOW_MS) {
    _windowStartMs += Timing::SSR_WINDOW_MS;
    if (now - _windowStartMs >= Timing::SSR_WINDOW_MS) {
      _windowStartMs = now;
    }
  }
  uint32_t onTime = static_cast<uint32_t>((_dutyPercent * Timing::SSR_WINDOW_MS) / 100.0f);
  writeOutput((now - _windowStartMs) < onTime);
}

void HeaterController::writeOutput(bool on) {
  _outputOn = on;
  writeSsrElectrical(on);
}

bool HeaterController::ssrPinHigh() const {
  return !_outputOn && readSsrPinHigh();
}

/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * heater.cpp: Heater output and PID time-window control
 */

#include "heater.h"
#include <math.h>

namespace {
#if SSR_ACTIVE_LOW
constexpr uint8_t SSR_DRIVER_ON_LEVEL = LOW;
constexpr uint8_t SSR_DRIVER_OFF_LEVEL = HIGH;
#else
constexpr uint8_t SSR_DRIVER_ON_LEVEL = HIGH;
constexpr uint8_t SSR_DRIVER_OFF_LEVEL = LOW;
#endif

constexpr float INTEGRAL_MIN = HeaterTuning::INTEGRAL_MIN;
constexpr float INTEGRAL_MAX = HeaterTuning::INTEGRAL_MAX;

constexpr float AMBIENT_BOOST_GAIN = HeaterTuning::AMBIENT_BOOST_GAIN;
constexpr float AMBIENT_BOOST_MAX = HeaterTuning::AMBIENT_BOOST_MAX;

constexpr float OVERSHOOT_CUTOFF_C = HeaterTuning::OVERSHOOT_CUTOFF_C;
constexpr float ABSOLUTE_MAX_PLATE_C = Limits::SAFETY_MAX_C;
constexpr float MIN_VALID_PLATE_C = HeaterTuning::MIN_VALID_PLATE_C;
constexpr float MAX_VALID_PLATE_C = HeaterTuning::MAX_VALID_PLATE_C;

constexpr float MAX_ALLOWED_TARGET_C = Limits::REFLOW_MAX_C;

constexpr float DUTY_MAX_STEP_PER_UPDATE = HeaterTuning::DUTY_MAX_STEP_PER_UPDATE;

bool readSsrCommandedOnFromPin() {
  return digitalRead(Pins::HEATER_CTRL) == SSR_DRIVER_ON_LEVEL;
}

void writeSsrElectrical(bool on) {
  digitalWrite(Pins::HEATER_CTRL, on ? SSR_DRIVER_ON_LEVEL : SSR_DRIVER_OFF_LEVEL);
}
}

float HeaterController::clampFloat(float value, float low, float high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

float HeaterController::slewLimit(float desired, float previous, float maxStep) {
  if (desired > previous + maxStep) return previous + maxStep;
  if (desired < previous - maxStep) return previous - maxStep;
  return desired;
}

void HeaterController::begin() {
  pinMode(Pins::HEATER_CTRL, OUTPUT);
  writeSsrElectrical(false);

  _enabled = false;
  _outputOn = false;
  _dutyPercent = 0.0f;
  _windowDutyPercent = 0.0f;
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
  _windowDutyPercent = 0.0f;
  _windowStartMs = millis();
  writeOutput(false);
}

void HeaterController::setEnabled(bool enabled) {
  if (_enabled == enabled) {
    return;
  }
  
  _enabled = enabled;

  if (enabled) {
    _integral = 0.0f;
    _hasLastInput = false;
    _lastPidMs = 0;
    _dutyPercent = 0.0f;
    _windowDutyPercent = 0.0f;
    _windowStartMs = millis();
    writeOutput(false);
  } 
  else {
    _dutyPercent = 0.0f;
    _windowDutyPercent = 0.0f;
    _integral = 0.0f;
    _hasLastInput = false;
    _lastPidMs = 0;
    writeOutput(false);
  }
}

void HeaterController::forceOff() {
  _enabled = false;
  _dutyPercent = 0.0f;
  _windowDutyPercent = 0.0f;
  _outputOn = false;

  _integral = 0.0f;
  _hasLastInput = false;
  _lastPidMs = 0;

  writeSsrElectrical(false);
}

float HeaterController::applyWarmupAssist(float output, float error) const {
  if (error > HeaterTuning::WARMUP_ERROR_FAR_C) {
    output = max(output, HeaterTuning::WARMUP_DUTY_FAR_PERCENT);
  } 
  else if (error > HeaterTuning::WARMUP_ERROR_MID_C) {
    output = max(output, HeaterTuning::WARMUP_DUTY_MID_PERCENT);
  } 
  else if (error > HeaterTuning::WARMUP_ERROR_NEAR_C) {
    output = max(output, HeaterTuning::WARMUP_DUTY_NEAR_PERCENT);
  }

  return output;
}

void HeaterController::updatePid(uint32_t now, float currentC, float targetC, float ambientC) {
  if (!_enabled) {
    return;
  }
  
  if (!isfinite(currentC) || !isfinite(targetC) || !isfinite(ambientC)) {
    forceOff();
    return;
  }

  if (currentC < MIN_VALID_PLATE_C || currentC > MAX_VALID_PLATE_C) {
    forceOff();
    return;
  }

  if (currentC >= ABSOLUTE_MAX_PLATE_C) {
    forceOff();
    return;
  }

  if (_lastPidMs != 0 && (now - _lastPidMs) < Timing::CONTROL_MS) {
    return;
  }

  if (targetC > MAX_ALLOWED_TARGET_C) {
    targetC = MAX_ALLOWED_TARGET_C;
  }

  float dt = _lastPidMs == 0 ? (Timing::CONTROL_MS / 1000.0f) : ((now - _lastPidMs) / 1000.0f);
  if (dt <= 0.0f || dt > 5.0f) {
    dt = Timing::CONTROL_MS / 1000.0f;
  }
  _lastPidMs = now;

  float error = targetC - currentC;
  
  float derivative = 0.0f;
  if (_hasLastInput) {
    derivative = (currentC - _lastInput) / dt;
  }

  _lastInput = currentC;
  _hasLastInput = true;

  float ambientBoost = clampFloat((targetC - ambientC) * AMBIENT_BOOST_GAIN, 0.0f, AMBIENT_BOOST_MAX);

  float proposedIntegral = clampFloat(_integral + (error * dt), INTEGRAL_MIN, INTEGRAL_MAX);

  float rawOutput =
    (_kp * error) +
    (_ki * proposedIntegral) -
    (_kd * derivative) +
    ambientBoost;

  rawOutput = applyWarmupAssist(rawOutput, error);

  if (currentC >= targetC + OVERSHOOT_CUTOFF_C) {
    rawOutput = 0.0f;
  }

  float limitedOutput = clampFloat(rawOutput, 0.0f, 100.0f);

  bool saturatedHigh = limitedOutput >= 100.0f;
  bool saturatedLow  = limitedOutput <= 0.0f;

  bool integralPushesHigh = error > 0.0f;
  bool integralPushesLow = error < 0.0f;

  if (!((saturatedHigh && integralPushesHigh) || (saturatedLow && integralPushesLow))) {
    _integral = proposedIntegral;
  }

  // Keep the SSR from backing off during an intentional ramp. The PID still
  // controls the final approach and hold zone, but a large remaining error
  // means the plate needs full heat rather than derivative-limited pulses.
  float desiredDuty = limitedOutput;
  _dutyPercent = slewLimit(desiredDuty, _dutyPercent, DUTY_MAX_STEP_PER_UPDATE);
  _dutyPercent = clampFloat(_dutyPercent, 0.0f, 100.0f);
}

void HeaterController::serviceWindow(uint32_t now) {
  if (!_enabled || _dutyPercent <= 0.0f) {
    _windowDutyPercent = 0.0f;
    writeOutput(false);
    return;
  }
  
  if (_windowDutyPercent <= 0.0f && _dutyPercent > 0.0f) {
    _windowDutyPercent = _dutyPercent;
  }

  if (now - _windowStartMs >= (Timing::SSR_WINDOW_MS * 4UL)) {
    _windowStartMs = now;
    _windowDutyPercent = _dutyPercent;
  }
  
  while (now - _windowStartMs >= Timing::SSR_WINDOW_MS) {
    _windowStartMs += Timing::SSR_WINDOW_MS;
    _windowDutyPercent = _dutyPercent;
  }

  uint32_t elapsed = now - _windowStartMs;
  uint32_t onTime = static_cast<uint32_t>((_windowDutyPercent * Timing::SSR_WINDOW_MS) / 100.0f);
  
  writeOutput(elapsed < onTime);
}

void HeaterController::writeOutput(bool on) {
  _outputOn = on;
  writeSsrElectrical(on);
}

bool HeaterController::ssrCommandedOn() const {
  return _outputOn && readSsrCommandedOnFromPin();
}
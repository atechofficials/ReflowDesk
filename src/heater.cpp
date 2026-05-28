/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * heater.cpp: Heater output and PID time-window control
 */

#include "heater.h"
#include <math.h>

namespace {
#if REFLOW_SSR_ACTIVE_LOW
constexpr uint8_t SSR_DRIVER_ON_LEVEL = LOW;
constexpr uint8_t SSR_DRIVER_OFF_LEVEL = HIGH;
#else
constexpr uint8_t SSR_DRIVER_ON_LEVEL = HIGH;
constexpr uint8_t SSR_DRIVER_OFF_LEVEL = LOW;
#endif

constexpr uint8_t HEATER_PWM_CHANNEL = 7;

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

bool attachHeaterPwm() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  return ledcAttach(Pins::HEATER_CTRL, HeaterTuning::DC_PWM_FREQ_HZ, HeaterTuning::DC_PWM_BITS);
#else
  ledcSetup(HEATER_PWM_CHANNEL, HeaterTuning::DC_PWM_FREQ_HZ, HeaterTuning::DC_PWM_BITS);
  ledcAttachPin(Pins::HEATER_CTRL, HEATER_PWM_CHANNEL);
  return true;
#endif
}

uint16_t dutyPercentToPwmCounts(float dutyPercent) {
  dutyPercent = constrain(dutyPercent, 0.0f, 100.0f);
  return static_cast<uint16_t>((dutyPercent * HeaterTuning::DC_PWM_MAX_DUTY + 50.0f) / 100.0f);
}

void writeHeaterPwmCounts(uint16_t counts) {
  if (counts > HeaterTuning::DC_PWM_MAX_DUTY) {
    counts = HeaterTuning::DC_PWM_MAX_DUTY;
  }
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(Pins::HEATER_CTRL, counts);
#else
  ledcWrite(HEATER_PWM_CHANNEL, counts);
#endif
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
#if REFLOW_HEATER_DC_PWM
  attachHeaterPwm();
  writeHeaterPwmCounts(0);
#else
  writeSsrElectrical(false);
#endif

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

  writeOutput(false);
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

#if REFLOW_HEATER_DC_PWM
  _windowDutyPercent = _dutyPercent;
  writeDutyPercent(_windowDutyPercent);
  return;
#endif
  
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
#if REFLOW_HEATER_DC_PWM
  writeHeaterPwmCounts(on ? HeaterTuning::DC_PWM_MAX_DUTY : 0);
#else
  writeSsrElectrical(on);
#endif
}

void HeaterController::writeDutyPercent(float dutyPercent) {
  dutyPercent = clampFloat(dutyPercent, 0.0f, 100.0f);
  _outputOn = dutyPercent > 0.0f;
#if REFLOW_HEATER_DC_PWM
  writeHeaterPwmCounts(dutyPercentToPwmCounts(dutyPercent));
#else
  writeSsrElectrical(_outputOn);
#endif
}

bool HeaterController::ssrCommandedOn() const {
#if REFLOW_HEATER_DC_PWM
  return _outputOn;
#else
  return _outputOn && readSsrCommandedOnFromPin();
#endif
}

const char *HeaterController::modeName() const {
#if REFLOW_HEATER_DC_PWM
  return "DC_PWM";
#else
  return "AC_SSR";
#endif
}

const char *HeaterController::outputLabel() const {
#if REFLOW_HEATER_DC_PWM
  return "MOSFET";
#else
  return "SSR";
#endif
}

uint32_t HeaterController::pwmFrequencyHz() const {
#if REFLOW_HEATER_DC_PWM
  return HeaterTuning::DC_PWM_FREQ_HZ;
#else
  return 0;
#endif
}

uint8_t HeaterController::pwmResolutionBits() const {
#if REFLOW_HEATER_DC_PWM
  return HeaterTuning::DC_PWM_BITS;
#else
  return 0;
#endif
}

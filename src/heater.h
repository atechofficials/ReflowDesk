/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * heater.h: Heater controller interface
 */

#pragma once

#include <Arduino.h>

#include "config.h"

class HeaterController {
public:
  void begin();
  void configure(float kp, float ki, float kd);
  void reset();
  void setEnabled(bool enabled);
  void forceOff();
  
  bool enabled() const { return _enabled; }
  
  void updatePid(uint32_t now, float currentC, float targetC, float ambientC);
  void serviceWindow(uint32_t now);

  float dutyPercent() const { return _dutyPercent; }
  float windowDutyPercent() const { return _windowDutyPercent; }
  
  bool outputOn() const { return _outputOn; }
  bool ssrCommandedOn() const;

private:
  void writeOutput(bool on);

  static float clampFloat(float value, float low, float high);
  static float slewLimit(float desired, float previous, float maxStep);

  float applyWarmupAssist(float output, float error) const;

  float _kp = 6.0f;
  float _ki = 0.04f;
  float _kd = 35.0f;

  float _integral = 0.0f;
  float _lastInput = 0.0f;
  bool _hasLastInput = false;
  
  bool _enabled = false;
  bool _outputOn = false;
  
  float _dutyPercent = 0.0f;
  float _windowDutyPercent = 0.0f;
  
  uint32_t _lastPidMs = 0;
  uint32_t _windowStartMs = 0;
};

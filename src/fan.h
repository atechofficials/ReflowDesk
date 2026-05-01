/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * fan.h: Cooling fan controller interface
 */

#pragma once

#include <Arduino.h>

#include "config.h"

class FanController {
public:
  void begin();
  void update(uint32_t now);
  void setSpeed(uint8_t percent);
  void stop();
  void full() { setSpeed(100); }
  uint8_t speedPercent() const { return _appliedPercent; }
  uint8_t requestedPercent() const { return _requestedPercent; }
  bool isOn() const { return _powerEnabled && _appliedPercent > 0; }
  bool powerEnabled() const { return _powerEnabled; }
  uint16_t rpm() const { return _rpm; }
  bool failed() const { return _failed; }

private:
  static void tachIsr();
  uint8_t constrainedDuty(uint8_t percent) const;
  void writeFanPower(bool enabled);
  void writeFanPwm(uint8_t fanHighPercent);
  void resetTachWindow(uint32_t now);

  static FanController *_instance;

  volatile uint32_t _tachPulses = 0;
  volatile uint32_t _tachRejected = 0;
  volatile uint32_t _lastTachUs = 0;
  uint32_t _lastRpmMs = 0;
  uint32_t _stallStartMs = 0;
  uint32_t _startupAssistUntilMs = 0;
  uint16_t _rpm = 0;
  uint8_t _requestedPercent = 0;
  uint8_t _appliedPercent = 0;
  bool _powerEnabled = false;
  bool _failed = false;
};

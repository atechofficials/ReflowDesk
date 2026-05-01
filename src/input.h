/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * input.h: Rotary encoder and button input interface
 */

#pragma once

#include <Arduino.h>
#include <RotaryEncoder.h>

#include "config.h"

struct InputEvent {
  int8_t rotation = 0;
  bool click = false;
};

class InputManager {
public:
  InputManager();
  void begin();
  void discardRotation();
  InputEvent poll(uint32_t now);

private:
  static void isr();
  RotaryEncoder _encoder;
  long _lastPosition = 0;
  bool _lastButton = HIGH;
  bool _stableButton = HIGH;
  uint32_t _lastDebounceMs = 0;
  static InputManager *_instance;
};

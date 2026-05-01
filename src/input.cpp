/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * input.cpp: Rotary encoder and button input handling
 */

#include "input.h"

InputManager *InputManager::_instance = nullptr;

InputManager::InputManager()
    : _encoder(Pins::ENCODER_A, Pins::ENCODER_B, RotaryEncoder::LatchMode::FOUR3) {}

void InputManager::begin() {
  _instance = this;
  pinMode(Pins::ENCODER_A, INPUT_PULLUP);
  pinMode(Pins::ENCODER_B, INPUT_PULLUP);
  pinMode(Pins::ENCODER_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Pins::ENCODER_A), InputManager::isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Pins::ENCODER_B), InputManager::isr, CHANGE);
  _lastPosition = _encoder.getPosition();
  _lastButton = digitalRead(Pins::ENCODER_BTN);
  _stableButton = _lastButton;
}

void InputManager::discardRotation() {
  _lastPosition = _encoder.getPosition();
}

InputEvent InputManager::poll(uint32_t now) {
  InputEvent event;
  long position = _encoder.getPosition();
  long delta = position - _lastPosition;
  if (delta != 0) {
    if (delta > 4) delta = 4;
    if (delta < -4) delta = -4;
    event.rotation = static_cast<int8_t>(delta);
    _lastPosition = position;
  }

  bool reading = digitalRead(Pins::ENCODER_BTN);
  if (reading != _lastButton) {
    _lastDebounceMs = now;
    _lastButton = reading;
  }
  if ((now - _lastDebounceMs) > Timing::BUTTON_DEBOUNCE_MS && reading != _stableButton) {
    _stableButton = reading;
    if (_stableButton == LOW) {
      event.click = true;
    }
  }
  return event;
}

void InputManager::isr() {
  if (_instance != nullptr) {
    _instance->_encoder.tick();
  }
}

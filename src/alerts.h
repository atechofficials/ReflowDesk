/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * alerts.h: Buzzer and status LED alert interface
 */

#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"
#include "settings.h"

class AlertManager {
public:
  void begin();
  void setBuzzerLevel(uint8_t level);
  void setLedBrightness(uint8_t brightness);
  void beep(uint16_t durationMs);
  void beepPattern(uint8_t count, uint16_t durationMs, uint16_t gapMs);
  void tick(uint32_t now);
  void updateStatusLed(float plateC, const SettingsData &settings, bool fault);

private:
  void setLed(uint8_t r, uint8_t g, uint8_t b);
  void attachBuzzerPwm();
  void writeBuzzer(bool enabled);
  void stopBuzzer();

  Adafruit_NeoPixel _led = Adafruit_NeoPixel(1, Pins::STATUS_LED, RGB_LED_COLOR_ORDER + NEO_KHZ800);
  uint8_t _buzzerLevel = 3;
  uint8_t _remainingBeeps = 0;
  uint16_t _beepMs = 0;
  uint16_t _gapMs = 0;
  bool _buzzing = false;
  uint32_t _nextToggleMs = 0;
};

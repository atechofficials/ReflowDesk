/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * sensors.h: Temperature sensor sampling interface
 */

#pragma once

#include <Arduino.h>
#include <ADS1X15.h>
#include <MAX6675.h>
#include <Wire.h>

#include "config.h"

struct TemperatureSample {
  float plateC = 0.0f;
  float ambientC = 25.0f;
  float boardC = 25.0f;
  bool plateOk = false;
  bool ambientOk = false;
  bool boardOk = false;
  bool plateReadOnce = false;
  bool ambientReadOnce = false;
  bool boardReadOnce = false;
  uint8_t plateStatus = 0;
  uint16_t plateRaw = 0;
  uint16_t ambientAdc = 0;
  uint16_t boardAdc = 0;
};

class SensorManager {
public:
  explicit SensorManager(TwoWire &wire);

  void begin();
  void update(uint32_t now, bool force = false);
  const TemperatureSample &sample() const { return _sample; }

private:
  void readPlate();
  void readAmbient();
  void readBoard();
  bool ensureAmbientAdc();
  void configureAmbientAdc();
  void readThermistor(uint8_t channel, float &temperatureC, bool &ok, bool &readOnce, uint16_t &adcRaw);
  float thermistorCelsius(float voltage, bool &ok) const;

  MAX6675 _plateThermo;
  ADS1115 _ambientAdc;
  TemperatureSample _sample;
  uint32_t _lastPlateMs = 0;
  uint32_t _lastAmbientMs = 0;
  bool _ambientAdcReady = false;
};

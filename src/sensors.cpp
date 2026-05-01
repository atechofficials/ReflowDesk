/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * sensors.cpp: Thermocouple and ambient sensor acquisition
 */

#include "sensors.h"

#include <math.h>

namespace {
constexpr uint8_t PLATE_STATUS_OK = 0x00;
constexpr uint8_t PLATE_STATUS_OPEN = 0x04;
constexpr uint8_t PLATE_STATUS_NO_COMM = 0x81;
}

SensorManager::SensorManager(TwoWire &wire)
    : _plateThermo(Pins::MAX6675_CS, Pins::MAX6675_SO, Pins::MAX6675_SCK),
      _ambientAdc(ADS1115_I2C_ADDRESS, &wire) {}

void SensorManager::begin() {
  _ambientAdcReady = _ambientAdc.begin();
  if (_ambientAdcReady) {
    _ambientAdc.setGain(ADS1X15_GAIN_4096MV);
    _ambientAdc.setMode(ADS1X15_MODE_SINGLE);
    _ambientAdc.setDataRate(ADS1X15_DATARATE_4);
  }
#if REFLOW_DEBUG
  Serial.print(F("ADS1115 addr=0x"));
  Serial.print(ADS1115_I2C_ADDRESS, HEX);
  Serial.print(F(" ready="));
  Serial.println(_ambientAdcReady ? F("yes") : F("no"));
#endif

  _plateThermo.begin();
  _plateThermo.setSWSPIdelay(MAX6675_SW_SPI_DELAY_US * 2);
  delay(500);
  update(millis(), true);
}

void SensorManager::update(uint32_t now, bool force) {
  if (force || now - _lastPlateMs >= Timing::TEMP_READ_MS) {
    readPlate();
    _lastPlateMs = now;
  }
  if (force || now - _lastAmbientMs >= Timing::AMBIENT_READ_MS) {
    readAmbient();
    _lastAmbientMs = now;
  }
}

void SensorManager::readPlate() {
  uint8_t status = _plateThermo.read();
  uint16_t raw = _plateThermo.getRawData();
  _sample.plateRaw = raw;
  _sample.plateReadOnce = true;

  if (status == STATUS_NO_COMMUNICATION || raw == 0xFFFF) {
    _sample.plateStatus = PLATE_STATUS_NO_COMM;
    _sample.plateOk = false;
    return;
  }

  if (status & STATUS_ERROR) {
    _sample.plateStatus = PLATE_STATUS_OPEN;
    _sample.plateOk = false;
    return;
  }

  float c = _plateThermo.getCelsius();
  if (isnan(c) || c < 0.0f || c > 350.0f) {
    _sample.plateStatus = PLATE_STATUS_NO_COMM;
    _sample.plateOk = false;
    return;
  }

  _sample.plateStatus = PLATE_STATUS_OK;
  _sample.plateOk = true;
  _sample.plateC = c;
}

void SensorManager::readAmbient() {
  if (!_ambientAdcReady) {
    _ambientAdcReady = _ambientAdc.begin();
    if (!_ambientAdcReady) {
      _sample.ambientReadOnce = true;
      _sample.ambientOk = false;
      _sample.ambientAdc = 0;
      return;
    }
    _ambientAdc.setGain(ADS1X15_GAIN_4096MV);
    _ambientAdc.setMode(ADS1X15_MODE_SINGLE);
    _ambientAdc.setDataRate(ADS1X15_DATARATE_4);
  }

  int16_t raw = _ambientAdc.readADC(ADS1115_NTC_CHANNEL);
  bool ok = false;
  float c = 0.0f;
  if (raw > 0) {
    float voltage = _ambientAdc.toVoltage(raw);
    c = thermistorCelsius(voltage, ok);
  }
  _sample.ambientAdc = raw > 0 ? static_cast<uint16_t>(raw) : 0;
  _sample.ambientReadOnce = true;
  _sample.ambientOk = ok;
  if (ok) {
    _sample.ambientC = c;
  }
}

float SensorManager::thermistorCelsius(float voltage, bool &ok) const {
  ok = false;
  if (voltage <= 0.005f || voltage >= (NTC_BIAS_VOLTAGE - 0.005f)) {
    return 0.0f;
  }
  float ratio = voltage / NTC_BIAS_VOLTAGE;
  float resistance = (NTC_SERIES_OHMS * ratio) / (1.0f - ratio);
  if (resistance < 1000.0f || resistance > 1000000.0f) {
    return 0.0f;
  }

  float steinhart = logf(resistance / NTC_NOMINAL_OHMS) / NTC_BETA;
  steinhart += 1.0f / (NTC_NOMINAL_TEMP_C + 273.15f);
  steinhart = (1.0f / steinhart) - 273.15f;
  ok = (steinhart > -20.0f && steinhart < 90.0f);
  return steinhart;
}

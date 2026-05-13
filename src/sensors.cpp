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
constexpr float AMBIENT_MAX_RISE_PER_READ_C = 1.0f;
constexpr float AMBIENT_MAX_FALL_PER_READ_C = 2.0f;
}

SensorManager::SensorManager(TwoWire &wire)
    : _plateThermo(Pins::MAX6675_CS, Pins::SPI_MISO, Pins::SPI_SCK),
      _ambientAdc(ADS1115_I2C_ADDRESS, &wire) {}

void SensorManager::begin() {
  _ambientAdcReady = _ambientAdc.begin();
  if (_ambientAdcReady) {
    configureAmbientAdc();
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
#if REFLOW_HAS_BOARD_NTC
    readBoard();
#endif
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
  float measuredC = _sample.ambientC;
  bool ok = false;
  bool readOnce = _sample.ambientReadOnce;
  uint16_t adcRaw = _sample.ambientAdc;
  readThermistor(ADS1115_AMBIENT_NTC_CHANNEL, measuredC, ok, readOnce, adcRaw);
  _sample.ambientReadOnce = readOnce;
  _sample.ambientOk = ok;
  _sample.ambientAdc = adcRaw;
  if (ok) {
    _sample.ambientC = filterAmbientC(measuredC);
  }
}

void SensorManager::readBoard() {
  readThermistor(ADS1115_BOARD_NTC_CHANNEL, _sample.boardC, _sample.boardOk, _sample.boardReadOnce,
                 _sample.boardAdc);
}

bool SensorManager::ensureAmbientAdc() {
  if (_ambientAdcReady) {
    return true;
  }

  _ambientAdcReady = _ambientAdc.begin();
  if (_ambientAdcReady) {
    configureAmbientAdc();
  }
  return _ambientAdcReady;
}

void SensorManager::configureAmbientAdc() {
  _ambientAdc.setGain(ADS1X15_GAIN_4096MV);
  _ambientAdc.setMode(ADS1X15_MODE_SINGLE);
  _ambientAdc.setDataRate(ADS1X15_DATARATE_4);
}

void SensorManager::readThermistor(uint8_t channel, float &temperatureC, bool &ok, bool &readOnce, uint16_t &adcRaw) {
  if (!ensureAmbientAdc()) {
    readOnce = true;
    ok = false;
    adcRaw = 0;
    return;
  }

  int16_t raw = _ambientAdc.readADC(channel);
  bool readingOk = false;
  float c = 0.0f;
  if (raw > 0) {
    float voltage = _ambientAdc.toVoltage(raw);
    c = thermistorCelsius(voltage, readingOk);
  }

  adcRaw = raw > 0 ? static_cast<uint16_t>(raw) : 0;
  readOnce = true;
  ok = readingOk;
  if (readingOk) {
    temperatureC = c;
  }
}

float SensorManager::filterAmbientC(float measuredC) {
  if (!_ambientFilterReady) {
    _ambientFilteredC = measuredC;
    _ambientFilterReady = true;
    return _ambientFilteredC;
  }

  float delta = measuredC - _ambientFilteredC;
  if (delta > AMBIENT_MAX_RISE_PER_READ_C) {
    delta = AMBIENT_MAX_RISE_PER_READ_C;
  } else if (delta < -AMBIENT_MAX_FALL_PER_READ_C) {
    delta = -AMBIENT_MAX_FALL_PER_READ_C;
  }
  _ambientFilteredC += delta;
  return _ambientFilteredC;
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

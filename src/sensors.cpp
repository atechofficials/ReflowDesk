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
    : _plateThermo(Pins::MAX6675_CS, Pins::SPI_MISO, Pins::SPI_SCK),
      _ambientAdc(ADS1115_I2C_ADDRESS, &wire) {}

void SensorManager::begin() {
  _ambientAdcReady = _ambientAdc.begin();
  if (_ambientAdcReady) {
    configureAdc();
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

  if (!isfinite(c) || c < SensorLimits::PLATE_SENSOR_MIN_VALID_C || c > SensorLimits::PLATE_SENSOR_MAX_VALID_C) {
    _sample.plateStatus = PLATE_STATUS_NO_COMM;
    _sample.plateOk = false;
    return;
  }

  // Hot Plate Temp Spike Filtering
  if (_hasLastGoodPlate) {
    float jump = fabsf(c - _lastGoodPlateC);

    if (jump > SensorLimits::PLATE_MAX_JUMP_PER_READ_C) {
      if (_plateSpikeCount < 255) {
        _plateSpikeCount++;
      }

      if (_plateSpikeCount < SensorLimits::PLATE_MAX_SPIKES_BEFORE_FAULT) {
        // Ignore this isolated spike and keep last known good value.
        _sample.plateStatus = PLATE_STATUS_OK;
        _sample.plateOk = true;
        _sample.plateC = _lastGoodPlateC;
        return;
      }

      _sample.plateStatus = PLATE_STATUS_NO_COMM;
      _sample.plateOk = false;
      return;
    }
  }

  _plateSpikeCount = 0;
  _hasLastGoodPlate = true;
  _lastGoodPlateC = c;

  _sample.plateStatus = PLATE_STATUS_OK;
  _sample.plateOk = true;
  _sample.plateC = c;
}

void SensorManager::readAmbient() {
  float measuredC = _sample.ambientC;
  bool ok = false;
  bool readOnce = _sample.ambientReadOnce;
  int16_t adcRaw = _sample.ambientAdc;
  readThermistor(ADS1115_AMBIENT_NTC_CHANNEL, measuredC, ok, readOnce, adcRaw);
  _sample.ambientReadOnce = readOnce;
  _sample.ambientOk = ok;
  _sample.ambientAdc = adcRaw;
  if (ok) {
    _ambientFailCount = 0;
    _sample.ambientC = filterAmbientC(measuredC);
  } 
  else {
    if (_ambientFailCount < 255) {
      _ambientFailCount++;
    }
    if (_ambientFailCount >= 3) {
      _sample.ambientC = 25.0f; // default/fallback value if ADC reading fails repeatedly
      _ambientFilterReady = false;
    }
  }
}

void SensorManager::readBoard() {
  float measuredC = _sample.boardC;
  bool ok = false;
  bool readOnce = _sample.boardReadOnce;
  int16_t adcRaw = _sample.boardAdc;
  
  readThermistor(ADS1115_BOARD_NTC_CHANNEL, measuredC, ok, readOnce, adcRaw);
  
  _sample.boardReadOnce = readOnce;
  _sample.boardOk = ok;
  _sample.boardAdc = adcRaw;

  if (ok) {
    _sample.boardC = filterBoardC(measuredC);
  }
}

bool SensorManager::ensureAdc() {
  if (_ambientAdcReady) {
    return true;
  }

  _ambientAdcReady = _ambientAdc.begin();
  if (_ambientAdcReady) {
    configureAdc();
  }
  return _ambientAdcReady;
}

void SensorManager::configureAdc() {
  _ambientAdc.setGain(ADS1X15_GAIN_4096MV);
  _ambientAdc.setMode(ADS1X15_MODE_SINGLE);
  _ambientAdc.setDataRate(ADS1X15_DATARATE_4);
}

void SensorManager::readThermistor(uint8_t channel, float &temperatureC, bool &ok, bool &readOnce, int16_t &adcRaw) {
  if (!ensureAdc()) {
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

  adcRaw = raw;
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
  if (delta > SensorLimits::AMBIENT_MAX_RISE_PER_READ_C) {
    delta = SensorLimits::AMBIENT_MAX_RISE_PER_READ_C;
  } else if (delta < -SensorLimits::AMBIENT_MAX_FALL_PER_READ_C) {
    delta = -SensorLimits::AMBIENT_MAX_FALL_PER_READ_C;
  }
  _ambientFilteredC += delta;
  return _ambientFilteredC;
}

float SensorManager::filterBoardC(float measuredC) {
  if (!_boardFilterReady) {
    _boardFilteredC = measuredC;
    _boardFilterReady = true;
    return _boardFilteredC;
  }

  float delta = measuredC - _boardFilteredC;

  if (delta > SensorLimits::BOARD_MAX_RISE_PER_READ_C) {
    delta = SensorLimits::BOARD_MAX_RISE_PER_READ_C;
  } else if (delta < -SensorLimits::BOARD_MAX_FALL_PER_READ_C) {
    delta = -SensorLimits::BOARD_MAX_FALL_PER_READ_C;
  }

  _boardFilteredC += delta;
  return _boardFilteredC;
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

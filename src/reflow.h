/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * reflow.h: Reflow process controller interface
 */

#pragma once

#include <Arduino.h>

#include "alerts.h"
#include "fan.h"
#include "heater.h"
#include "sensors.h"
#include "settings.h"

enum class ReflowState : uint8_t {
  Idle,
  Preheat,
  Soak,
  Reflow,
  Cooling,
  Fault,
  Aborted
};

enum class FaultReason : uint8_t {
  None,
  PlateSensor,
  AmbientSensor,
  OverTemperature,
  HeaterNoRise,
  FanFailure,
  UserAbort
};

class ReflowController {
public:
  ReflowController(HeaterController &heater, FanController &fan, AlertManager &alerts);

  void begin();
  bool start(const SettingsData &settings, const TemperatureSample &sample);
  void abort();
  void acknowledge(const SettingsData &settings, const TemperatureSample &sample);
  void update(uint32_t now, const SettingsData &settings, const TemperatureSample &sample);

  ReflowState state() const { return _state; }
  FaultReason faultReason() const { return _faultReason; }
  bool isActive() const;
  bool isHeatingState() const;
  bool isFaultLike() const { return _state == ReflowState::Fault || _state == ReflowState::Aborted; }
  bool canStart(const SettingsData &settings, const TemperatureSample &sample) const;
  bool canAcknowledge(const SettingsData &settings, const TemperatureSample &sample, uint32_t now) const;
  bool cooldownLocked(const SettingsData &settings, const TemperatureSample &sample, uint32_t now) const;
  uint16_t cooldownRemainingSeconds(uint32_t now) const;
  float targetC(const SettingsData &settings) const;
  uint32_t holdElapsedSeconds(uint32_t now) const;
  uint16_t holdTargetSeconds(const SettingsData &settings) const;
  bool holding() const { return _holdStartMs != 0; }
  const char *stateName() const;
  const char *faultName() const;

private:
  void enterState(ReflowState state, uint32_t now);
  void enterFault(FaultReason reason, uint32_t now);
  void updateCooling(const SettingsData &settings, float plateC);
  bool safetyTrip(const SettingsData &settings, const TemperatureSample &sample);
  bool heaterResponseTrip(uint32_t now, const SettingsData &settings, const TemperatureSample &sample);
  bool timedCooldownFault() const;
  bool cooldownComplete(const SettingsData &settings, const TemperatureSample &sample, uint32_t now) const;
  void keepFanOn(uint32_t now);

  HeaterController &_heater;
  FanController &_fan;
  AlertManager &_alerts;
  ReflowState _state = ReflowState::Idle;
  FaultReason _faultReason = FaultReason::None;
  uint32_t _stageStartMs = 0;
  uint32_t _holdStartMs = 0;
  uint32_t _lastControlMs = 0;
  uint32_t _heatWatchStartMs = 0;
  uint32_t _forcedFanUntilMs = 0;
  float _heatWatchStartC = 0.0f;
};

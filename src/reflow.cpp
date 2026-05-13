/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * reflow.cpp: Reflow process state machine and safety logic
 */

#include "reflow.h"

ReflowController::ReflowController(HeaterController &heater, FanController &fan, AlertManager &alerts)
    : _heater(heater), _fan(fan), _alerts(alerts) {}

void ReflowController::begin() {
  _heater.forceOff();
  _fan.stop();
}

bool ReflowController::start(const SettingsData &settings, const TemperatureSample &sample) {
  if (!sample.plateOk || !sample.ambientOk || sample.plateC >= settings.safetyCutoffC) {
    enterFault(!sample.plateOk ? FaultReason::PlateSensor : (!sample.ambientOk ? FaultReason::AmbientSensor : FaultReason::OverTemperature), millis());
    return false;
  }
  if (!canStart(settings, sample)) {
    return false;
  }
  _faultReason = FaultReason::None;
  _forcedFanUntilMs = 0;
  _heater.configure(SettingsStore::kp(settings), SettingsStore::ki(settings), SettingsStore::kd(settings));
  _heater.reset();
  _heater.setEnabled(true);
  enterState(ReflowState::Preheat, millis());
  _alerts.beepPattern(2, 80, 80);
  return true;
}

void ReflowController::abort() {
  if (!isActive()) {
    return;
  }
  _heater.forceOff();
  _fan.full();
  _state = ReflowState::Aborted;
  _faultReason = FaultReason::UserAbort;
  _holdStartMs = 0;
  _alerts.beepPattern(3, 120, 100);
}

void ReflowController::acknowledge(const SettingsData &settings, const TemperatureSample &sample) {
  if (_state == ReflowState::Fault || _state == ReflowState::Aborted) {
    if (!canAcknowledge(settings, sample, millis())) {
      return;
    }
    _state = ReflowState::Idle;
    _faultReason = FaultReason::None;
    _heater.forceOff();
    _fan.stop();
  }
}

void ReflowController::update(uint32_t now, const SettingsData &settings, const TemperatureSample &sample) {
  _fan.update(now);
  if (_fan.failed() && _faultReason != FaultReason::FanFailure) {
    enterFault(FaultReason::FanFailure, now);
  }

  if (!isHeatingState()) {
    _heater.forceOff();
  }

  if (_state == ReflowState::Fault || _state == ReflowState::Aborted) {
    _heater.forceOff();
    if (_state == ReflowState::Aborted && sample.plateReadOnce && !sample.plateOk) {
      enterFault(FaultReason::PlateSensor, now);
    }
    if (cooldownComplete(settings, sample, now)) {
      _fan.stop();
      if (_state == ReflowState::Aborted) {
        _state = ReflowState::Idle;
        _faultReason = FaultReason::None;
      }
    } else {
      _fan.full();
    }
    return;
  }

  if (safetyTrip(settings, sample)) {
    _heater.forceOff();
    return;
  }

  if (_state == ReflowState::Idle) {
    _heater.forceOff();
    if (sample.plateOk && sample.plateC > settings.safeTouchC) {
      updateCooling(settings, sample.plateC);
    } else {
      _fan.stop();
    }
    return;
  }

  if (_state == ReflowState::Cooling) {
    _heater.forceOff();
    updateCooling(settings, sample.plateC);
    if (sample.plateOk && sample.plateC <= settings.safeTouchC) {
      _fan.stop();
      _state = ReflowState::Idle;
    }
    return;
  }

  float target = targetC(settings);
  _heater.setEnabled(true);
  if (now - _lastControlMs >= Timing::CONTROL_MS) {
    _heater.updatePid(now, sample.plateC, target, sample.ambientC);
    _lastControlMs = now;
  }

  if (heaterResponseTrip(now, settings, sample)) {
    _heater.forceOff();
    return;
  }

  if (_holdStartMs == 0 && sample.plateC >= target - 3.0f) {
    _holdStartMs = now;
    _alerts.beep(80);
  }

  uint16_t holdTarget = holdTargetSeconds(settings);
  if (_holdStartMs != 0 && ((now - _holdStartMs) / 1000UL) >= holdTarget) {
    if (_state == ReflowState::Preheat) {
      enterState(ReflowState::Soak, now);
    } else if (_state == ReflowState::Soak) {
      enterState(ReflowState::Reflow, now);
    } else if (_state == ReflowState::Reflow) {
      enterState(ReflowState::Cooling, now);
      _heater.forceOff();
      updateCooling(settings, sample.plateC);
      _alerts.beepPattern(4, 140, 120);
    }
  }

  if (isHeatingState()) {
    _heater.serviceWindow(now);
  } else {
    _heater.forceOff();
  }
}

bool ReflowController::isActive() const {
  return isHeatingState() || _state == ReflowState::Cooling;
}

bool ReflowController::isHeatingState() const {
  return _state == ReflowState::Preheat || _state == ReflowState::Soak || _state == ReflowState::Reflow;
}

bool ReflowController::canStart(const SettingsData &settings, const TemperatureSample &sample) const {
  return _state == ReflowState::Idle && sample.plateOk && sample.ambientOk && sample.plateC <= settings.safeTouchC &&
         sample.plateC < settings.safetyCutoffC;
}

bool ReflowController::canAcknowledge(const SettingsData &settings, const TemperatureSample &sample, uint32_t now) const {
  return _state == ReflowState::Fault && cooldownComplete(settings, sample, now);
}

bool ReflowController::cooldownLocked(const SettingsData &settings, const TemperatureSample &sample, uint32_t now) const {
  if (_state == ReflowState::Cooling) {
    return true;
  }
  if (_state == ReflowState::Fault || _state == ReflowState::Aborted) {
    return !cooldownComplete(settings, sample, now);
  }
  return _state == ReflowState::Idle && (!sample.plateOk || sample.plateC > settings.safeTouchC);
}

uint16_t ReflowController::cooldownRemainingSeconds(uint32_t now) const {
  if (!timedCooldownFault() || static_cast<int32_t>(_forcedFanUntilMs - now) <= 0) {
    return 0;
  }
  return static_cast<uint16_t>((_forcedFanUntilMs - now + 999UL) / 1000UL);
}

float ReflowController::targetC(const SettingsData &settings) const {
  const ReflowProfile &profile = SettingsStore::activeProfile(settings);
  switch (_state) {
    case ReflowState::Preheat:
      return profile.preheatTempC;
    case ReflowState::Soak:
      return profile.soakTempC;
    case ReflowState::Reflow:
      return profile.reflowTempC;
    case ReflowState::Cooling:
      return settings.safeTouchC;
    default:
      return 0.0f;
  }
}

uint32_t ReflowController::holdElapsedSeconds(uint32_t now) const {
  if (_holdStartMs == 0) {
    return 0;
  }
  return (now - _holdStartMs) / 1000UL;
}

uint16_t ReflowController::holdTargetSeconds(const SettingsData &settings) const {
  const ReflowProfile &profile = SettingsStore::activeProfile(settings);
  switch (_state) {
    case ReflowState::Preheat:
      return profile.preheatSeconds;
    case ReflowState::Soak:
      return profile.soakSeconds;
    case ReflowState::Reflow:
      return profile.reflowSeconds;
    default:
      return 0;
  }
}

const char *ReflowController::stateName() const {
  switch (_state) {
    case ReflowState::Idle:
      return "Idle";
    case ReflowState::Preheat:
      return "Preheat";
    case ReflowState::Soak:
      return "Soak";
    case ReflowState::Reflow:
      return "Reflow";
    case ReflowState::Cooling:
      return "Cooling";
    case ReflowState::Fault:
      return "Fault";
    case ReflowState::Aborted:
      return "Aborted";
  }
  return "?";
}

const char *ReflowController::faultName() const {
  switch (_faultReason) {
    case FaultReason::PlateSensor:
      return "Plate sensor";
    case FaultReason::AmbientSensor:
      return "Ambient sensor";
    case FaultReason::OverTemperature:
      return "Over temp";
    case FaultReason::HeaterNoRise:
      return "Thermo stuck";
    case FaultReason::FanFailure:
      return "Fan failure";
    case FaultReason::UserAbort:
      return "User abort";
    default:
      return "None";
  }
}

void ReflowController::enterState(ReflowState state, uint32_t now) {
  _state = state;
  _stageStartMs = now;
  _holdStartMs = 0;
  _lastControlMs = 0;
  _heatWatchStartMs = 0;
  _heater.reset();
  _heater.setEnabled(state == ReflowState::Preheat || state == ReflowState::Soak || state == ReflowState::Reflow);
  if (state == ReflowState::Cooling) {
    _forcedFanUntilMs = 0;
  }
  _alerts.beep(70);
}

void ReflowController::enterFault(FaultReason reason, uint32_t now) {
  _heater.forceOff();
  _fan.full();
  if (_state == ReflowState::Fault && _faultReason == reason) {
    return;
  }
  _state = ReflowState::Fault;
  _faultReason = reason;
  _holdStartMs = 0;
  _heatWatchStartMs = 0;
  if (timedCooldownFault()) {
    keepFanOn(now);
  } else {
    _forcedFanUntilMs = 0;
  }
  _alerts.beepPattern(5, 120, 100);
}

void ReflowController::updateCooling(const SettingsData &settings, float plateC) {
  const ReflowProfile &profile = SettingsStore::activeProfile(settings);
  uint8_t speed = 35;
  switch (profile.coolingProfile) {
    case COOLING_PROFILE_RAPID:
      if (plateC > 70.0f) {
        speed = 100;
      } else if (plateC > settings.safeTouchC + 10) {
        speed = 50;
      } else {
        speed = 35;
      }
      break;
    case COOLING_PROFILE_SILENT:
      speed = 25;
      if (plateC >= 170.0f || plateC > settings.safeTouchC + 120) {
        speed = 75;
      } else if (plateC >= 140.0f || plateC > settings.safeTouchC + 80) {
        speed = 65;
      } else if (plateC >= 100.0f || plateC > settings.safeTouchC + 45) {
        speed = 50;
      } else if (plateC >= 70.0f || plateC > settings.safeTouchC + 20) {
        speed = 35;
      }
      break;
    case COOLING_PROFILE_NORMAL:
    default:
      if (plateC >= 170.0f || plateC > settings.safeTouchC + 120) {
        speed = 100;
      } else if (plateC >= 140.0f || plateC > settings.safeTouchC + 80) {
        speed = 90;
      } else if (plateC >= 100.0f || plateC > settings.safeTouchC + 45) {
        speed = 75;
      } else if (plateC >= 70.0f || plateC > settings.safeTouchC + 20) {
        speed = 55;
      }
      break;
  }
  _fan.setSpeed(speed);
}

bool ReflowController::safetyTrip(const SettingsData &settings, const TemperatureSample &sample) {
  if (sample.plateReadOnce && !sample.plateOk) {
    enterFault(FaultReason::PlateSensor, millis());
    return true;
  }
  if (sample.ambientReadOnce && !sample.ambientOk) {
    enterFault(FaultReason::AmbientSensor, millis());
    return true;
  }
  if (sample.plateOk && sample.plateC >= settings.safetyCutoffC) {
    enterFault(FaultReason::OverTemperature, millis());
    return true;
  }
  return false;
}

bool ReflowController::heaterResponseTrip(uint32_t now, const SettingsData &settings, const TemperatureSample &sample) {
  if (_state != ReflowState::Preheat && _state != ReflowState::Soak && _state != ReflowState::Reflow) {
    _heatWatchStartMs = 0;
    return false;
  }

  bool shouldHeatHard = _heater.dutyPercent() >= 70.0f && sample.plateOk && sample.plateC < targetC(settings) - 20.0f;
  if (!shouldHeatHard) {
    _heatWatchStartMs = 0;
    return false;
  }

  if (_heatWatchStartMs == 0) {
    _heatWatchStartMs = now;
    _heatWatchStartC = sample.plateC;
    return false;
  }

  if (now - _heatWatchStartMs < Timing::HEATER_RESPONSE_CHECK_MS) {
    return false;
  }

  if (sample.plateC - _heatWatchStartC < MIN_HEATER_RESPONSE_RISE_C) {
    enterFault(FaultReason::HeaterNoRise, now);
    return true;
  }

  _heatWatchStartMs = now;
  _heatWatchStartC = sample.plateC;
  return false;
}

bool ReflowController::timedCooldownFault() const {
  return _faultReason == FaultReason::PlateSensor || _faultReason == FaultReason::HeaterNoRise;
}

bool ReflowController::cooldownComplete(const SettingsData &settings, const TemperatureSample &sample, uint32_t now) const {
  if (timedCooldownFault()) {
    return static_cast<int32_t>(now - _forcedFanUntilMs) >= 0;
  }
  return sample.plateOk && sample.plateC <= settings.safeTouchC;
}

void ReflowController::keepFanOn(uint32_t now) {
  uint32_t until = now + Timing::FORCED_COOLDOWN_MS;
  if (static_cast<int32_t>(until - _forcedFanUntilMs) > 0) {
    _forcedFanUntilMs = until;
  }
}

/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * ui.cpp: OLED user interface screens and settings editor
 */

#include "ui.h"

namespace {
constexpr uint8_t SETTINGS_COUNT = 16;
constexpr uint8_t SETTINGS_VISIBLE = 5;

enum SettingIndex : uint8_t {
  IDX_PREHEAT_TEMP,
  IDX_PREHEAT_TIME,
  IDX_SOAK_TEMP,
  IDX_SOAK_TIME,
  IDX_REFLOW_TEMP,
  IDX_REFLOW_TIME,
  IDX_SAFE_TOUCH,
  IDX_SAFETY_CUTOFF,
  IDX_BUZZER,
  IDX_COOLING_PROFILE,
  IDX_KP,
  IDX_KI,
  IDX_KD,
  IDX_RESET,
  IDX_ABOUT,
  IDX_BACK
};

template <typename T>
T clampLocal(T value, T low, T high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

void printTempValue(Adafruit_SSD1306 &display, float value, bool ok, uint8_t decimals) {
  if (ok) {
    display.print(value, decimals);
    display.print(F("C"));
  } else {
    display.print(F("--.-C"));
  }
}
}

UiManager::UiManager(TwoWire &wire)
    : _display(OLED_WIDTH, OLED_HEIGHT, &wire, OLED_RESET) {}

bool UiManager::begin() {
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    return false;
  }
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println(F("SMD Reflow Plate"));
  _display.println(F("Controller"));
  _display.println();
  _display.println(F("Initializing..."));
  _display.display();
  return true;
}

void UiManager::handleInput(const InputEvent &event, SettingsStore &settings, ReflowController &reflow,
                            AlertManager &alerts, const TemperatureSample &sample) {
  if (event.rotation == 0 && !event.click) {
    return;
  }
  if (event.rotation != 0 || event.click) {
    alerts.beep(35);
  }

  if (reflow.isHeatingState() && event.click) {
    reflow.abort();
    _screen = Screen::Home;
    _homeSelection = 0;
    return;
  }

  uint32_t now = millis();
  const SettingsData &current = settings.data();
  if (_screen != Screen::Home && (reflow.state() != ReflowState::Idle || reflow.cooldownLocked(current, sample, now))) {
    _screen = Screen::Home;
    _homeSelection = 0;
    return;
  }
  switch (_screen) {
    case Screen::Home:
      if (reflow.isFaultLike()) {
        _homeSelection = 0;
        if (event.click && reflow.canAcknowledge(current, sample, now)) {
          reflow.acknowledge(current, sample);
        }
        break;
      }
      if (reflow.state() == ReflowState::Cooling || reflow.cooldownLocked(current, sample, now)) {
        _homeSelection = 0;
        break;
      }
      if (event.rotation != 0) {
        _homeSelection = (_homeSelection == 0) ? 1 : 0;
      }
      if (event.click) {
        if (_homeSelection == 0) {
          if (reflow.canStart(current, sample)) {
            reflow.start(settings.data(), sample);
          }
        } else {
          beginSettingsDraft(current);
          _screen = Screen::Settings;
        }
      }
      break;

    case Screen::Settings:
      if (event.rotation != 0) {
        int next = static_cast<int>(_settingsSelection) + event.rotation;
        _settingsSelection = clampLocal<int>(next, 0, SETTINGS_COUNT - 1);
        keepSelectionVisible();
      }
      if (event.click) {
        if (_settingsSelection == IDX_BACK) {
          _draftActive = false;
          _screen = Screen::Home;
        } else if (_settingsSelection == IDX_ABOUT) {
          _screen = Screen::About;
        } else if (_settingsSelection == IDX_RESET) {
          _homeSelection = 0;
          _screen = Screen::ResetConfirm;
        } else {
          _screen = Screen::Edit;
        }
      }
      break;

    case Screen::Edit:
      if (event.rotation != 0) {
        if (!_draftActive) {
          beginSettingsDraft(current);
        }
        adjustSetting(_draftSettings, event.rotation, _settingsSelection);
      }
      if (event.click) {
        commitSettingsDraft(settings);
        alerts.setBuzzerEnabled(settings.data().buzzerEnabled != 0);
        alerts.setLedBrightness(settings.data().ledBrightness);
        _screen = Screen::Settings;
      }
      break;

    case Screen::About:
      if (event.click) {
        _screen = Screen::Settings;
      }
      break;

    case Screen::ResetConfirm:
      if (event.rotation != 0) {
        _homeSelection = (_homeSelection == 0) ? 1 : 0;
      }
      if (event.click) {
        if (_homeSelection == 0) {
          settings.resetDefaults();
          settings.save();
          beginSettingsDraft(settings.data());
          alerts.setBuzzerEnabled(settings.data().buzzerEnabled != 0);
          alerts.setLedBrightness(settings.data().ledBrightness);
        }
        _screen = Screen::Settings;
      }
      break;
  }
}

void UiManager::draw(uint32_t now, const SettingsStore &settings, const ReflowController &reflow,
                     const HeaterController &heater, const FanController &fan, const TemperatureSample &sample,
                     const FanController *boardFan) {
  if (now - _lastDrawMs < Timing::DISPLAY_MS) {
    return;
  }
  _lastDrawMs = now;
  if (_screen != Screen::Home && (reflow.state() != ReflowState::Idle || reflow.cooldownLocked(settings.data(), sample, now))) {
    _screen = Screen::Home;
    _homeSelection = 0;
  }
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);

  switch (_screen) {
    case Screen::Home:
      drawHome(settings.data(), reflow, heater, fan, sample, boardFan);
      break;
    case Screen::Settings:
      drawSettings(settings);
      break;
    case Screen::Edit:
      drawEdit(_draftActive ? _draftSettings : settings.data());
      break;
    case Screen::About:
      drawAbout();
      break;
    case Screen::ResetConfirm:
      drawResetConfirm();
      break;
  }
  _display.display();
}

void UiManager::drawHome(const SettingsData &settings, const ReflowController &reflow,
                         const HeaterController &heater, const FanController &fan, const TemperatureSample &sample,
                         const FanController *boardFan) {
  _display.setCursor(0, 0);
  _display.print(F("Plate "));
  printTempValue(_display, sample.plateC, sample.plateOk, 1);
  _display.setCursor(82, 0);
  _display.print(F("Amb "));
  if (sample.ambientOk) {
    _display.print(sample.ambientC, 0);
    _display.print(F("C"));
  } else {
    _display.print(F("--"));
  }

  _display.setCursor(0, 12);
  _display.print(F("Stage: "));
  _display.print(reflow.stateName());
  bool boardFanFailed = boardFan != nullptr && boardFan->failed();
  if (reflow.isFaultLike()) {
    _display.setCursor(0, 22);
    _display.print(F("Reason: "));
    _display.print(reflow.faultName());
    _display.setCursor(0, 32);
    uint16_t remain = reflow.cooldownRemainingSeconds(millis());
    if (remain > 0) {
      _display.print(F("Fan on "));
      _display.print(remain);
      _display.print(F("s"));
    } else if (sample.plateOk) {
      _display.print(F("Cooling to "));
      _display.print(settings.safeTouchC);
      _display.print(F("C"));
    } else {
      _display.print(F("Cooling fan on"));
    }
  } else if (boardFanFailed) {
    _display.setCursor(0, 22);
    _display.print(F("Motherboard Cooling"));
    _display.setCursor(0, 32);
    _display.print(F("Fan Failed"));
  } else if (reflow.isActive() && reflow.state() != ReflowState::Cooling) {
    uint32_t elapsed = reflow.holdElapsedSeconds(millis());
    uint16_t total = reflow.holdTargetSeconds(settings);
    _display.setCursor(0, 22);
    _display.print(F("Set "));
    _display.print(reflow.targetC(settings), 0);
    _display.print(F("C "));
    _display.print(reflow.holding() ? F("Hold ") : F("Ramp "));
    _display.print(elapsed);
    _display.print(F("/"));
    _display.print(total);
    _display.print(F("s"));
  } else if (reflow.state() == ReflowState::Cooling) {
    _display.setCursor(0, 22);
    _display.print(F("Safe at "));
    _display.print(settings.safeTouchC);
    _display.print(F("C"));
  } else if (sample.plateOk && sample.plateC > settings.safeTouchC) {
    _display.setCursor(0, 22);
    _display.print(F("Cooling to "));
    _display.print(settings.safeTouchC);
    _display.print(F("C"));
  }

  uint8_t statusY = reflow.isFaultLike() ? 42 : (boardFanFailed ? 44 : 34);
  _display.setCursor(0, statusY);
  _display.print(F("H:"));
  _display.print(heater.outputOn() ? F("ON ") : F("OFF"));
  _display.print(F(" "));
  _display.print(heater.dutyPercent(), 0);
  _display.print(F("% P:"));
  _display.print(heater.ssrPinHigh() ? F("H") : F("L"));
  _display.print(F(" F:"));
  _display.print(fan.speedPercent());
  _display.print(F("%"));

  bool mainMenuReady = reflow.state() == ReflowState::Idle && reflow.canStart(settings, sample);
  bool canAck = reflow.canAcknowledge(settings, sample, millis());
  uint8_t actionY = reflow.isFaultLike() ? 54 : (boardFanFailed ? 56 : 48);
  _display.setCursor(0, actionY);

  if (reflow.isHeatingState()) {
    _display.print(F(">"));
    _display.print(F("Abort"));
  } else if (reflow.isFaultLike()) {
    if (canAck) {
      _display.print(F(">Ack"));
    } else {
      _display.print(F("Cooling..."));
    }
  } else if (boardFanFailed) {
    _display.print(F("Check board fan"));
  } else if (reflow.state() == ReflowState::Cooling || !mainMenuReady) {
    _display.print(F("Cooling..."));
  } else {
    _display.print(_homeSelection == 0 ? F(">") : F(" "));
    _display.print(F("Start"));
    _display.setCursor(70, 48);
    _display.print(_homeSelection == 1 ? F(">") : F(" "));
    _display.print(F("Settings"));
  }
}

void UiManager::drawSettings(const SettingsStore &settings) {
  const SettingsData &visibleSettings = _draftActive ? _draftSettings : settings.data();
  _display.setCursor(0, 0);
  _display.print(F("Settings"));
  _display.setCursor(92, 0);
  _display.print(settings.storageName());
  _display.print(settings.isStorageReady() ? F(":OK") : F(":--"));

  for (uint8_t row = 0; row < SETTINGS_VISIBLE; ++row) {
    uint8_t index = _settingsTop + row;
    if (index >= SETTINGS_COUNT) {
      break;
    }
    uint8_t y = 12 + row * 10;
    _display.setCursor(0, y);
    _display.print(index == _settingsSelection ? F(">") : F(" "));
    _display.print(settingLabel(index));
    _display.setCursor(82, y);
    printSettingValue(visibleSettings, index);
  }
}

void UiManager::drawEdit(const SettingsData &settings) {
  _display.setCursor(0, 0);
  _display.print(F("Edit"));
  _display.setCursor(0, 18);
  _display.print(settingLabel(_settingsSelection));
  _display.setTextSize(2);
  _display.setCursor(0, 34);
  printSettingValue(settings, _settingsSelection);
  _display.setTextSize(1);
  drawFooter(F("Turn=change"), F("Click=save"));
}

void UiManager::drawAbout() {
  _display.setCursor(0, 0);
  _display.print(DeviceInfo::NAME);
  _display.setCursor(0, 12);
  _display.print(F("Manufacturer"));
  _display.setCursor(0, 22);
  _display.print(DeviceInfo::MANUFACTURER);
  _display.setCursor(0, 34);
  _display.print(F("Model: "));
  _display.print(DeviceInfo::MODEL);
  _display.setCursor(0, 46);
  _display.print(F("HW v"));
  _display.print(DeviceInfo::HW_VERSION);
  _display.setCursor(64, 46);
  _display.print(F("FW v"));
  _display.print(DeviceInfo::FW);
  drawFooter(F(""), F("Click=back"));
}

void UiManager::drawResetConfirm() {
  _display.setCursor(0, 0);
  _display.println(F("Reset settings?"));
  _display.setCursor(0, 24);
  _display.print(_homeSelection == 0 ? F(">") : F(" "));
  _display.print(F("Yes"));
  _display.setCursor(70, 24);
  _display.print(_homeSelection == 1 ? F(">") : F(" "));
  _display.print(F("No"));
}

void UiManager::drawFooter(const __FlashStringHelper *left, const __FlashStringHelper *right) {
  _display.setTextSize(1);
  _display.setCursor(0, 56);
  _display.print(left);
  _display.setCursor(68, 56);
  _display.print(right);
}

const char *UiManager::settingLabel(uint8_t index) const {
  static const char *const labels[] = {
      "Preheat C", "Preheat s", "Soak C", "Soak s", "Reflow C", "Reflow s", "Safe C", "Cutoff C",
      "Buzzer", "Cooling", "Kp", "Ki", "Kd", "Reset", "About", "Back"};
  return labels[index];
}

void UiManager::printSettingValue(const SettingsData &settings, uint8_t index) {
  switch (index) {
    case IDX_PREHEAT_TEMP:
      _display.print(settings.preheatTempC);
      _display.print(F("C"));
      break;
    case IDX_PREHEAT_TIME:
      _display.print(settings.preheatSeconds);
      _display.print(F("s"));
      break;
    case IDX_SOAK_TEMP:
      _display.print(settings.soakTempC);
      _display.print(F("C"));
      break;
    case IDX_SOAK_TIME:
      _display.print(settings.soakSeconds);
      _display.print(F("s"));
      break;
    case IDX_REFLOW_TEMP:
      _display.print(settings.reflowTempC);
      _display.print(F("C"));
      break;
    case IDX_REFLOW_TIME:
      _display.print(settings.reflowSeconds);
      _display.print(F("s"));
      break;
    case IDX_SAFE_TOUCH:
      _display.print(settings.safeTouchC);
      _display.print(F("C"));
      break;
    case IDX_SAFETY_CUTOFF:
      _display.print(settings.safetyCutoffC);
      _display.print(F("C"));
      break;
    case IDX_BUZZER:
      _display.print(settings.buzzerEnabled ? F("On") : F("Off"));
      break;
    case IDX_COOLING_PROFILE:
      _display.print(SettingsStore::coolingProfileName(settings.coolingProfile));
      break;
    case IDX_KP:
      _display.print(SettingsStore::kp(settings), 2);
      break;
    case IDX_KI:
      _display.print(SettingsStore::ki(settings), 2);
      break;
    case IDX_KD:
      _display.print(SettingsStore::kd(settings), 2);
      break;
    case IDX_RESET:
    case IDX_ABOUT:
    case IDX_BACK:
      _display.print(F(">"));
      break;
  }
}

void UiManager::adjustSetting(SettingsData &settings, int8_t delta, uint8_t index) {
  switch (index) {
    case IDX_PREHEAT_TEMP:
      settings.preheatTempC = clampLocal<int16_t>(settings.preheatTempC + delta, Limits::PREHEAT_MIN_C, Limits::PREHEAT_MAX_C);
      break;
    case IDX_PREHEAT_TIME:
      settings.preheatSeconds = static_cast<uint16_t>(clampLocal<int>(static_cast<int>(settings.preheatSeconds) + delta * 5,
                                                                      Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S));
      break;
    case IDX_SOAK_TEMP:
      settings.soakTempC = clampLocal<int16_t>(settings.soakTempC + delta, Limits::SOAK_MIN_C, Limits::SOAK_MAX_C);
      break;
    case IDX_SOAK_TIME:
      settings.soakSeconds = static_cast<uint16_t>(clampLocal<int>(static_cast<int>(settings.soakSeconds) + delta * 5,
                                                                   Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S));
      break;
    case IDX_REFLOW_TEMP:
      settings.reflowTempC = clampLocal<int16_t>(settings.reflowTempC + delta, Limits::REFLOW_MIN_C, Limits::REFLOW_MAX_C);
      break;
    case IDX_REFLOW_TIME:
      settings.reflowSeconds = static_cast<uint16_t>(clampLocal<int>(static_cast<int>(settings.reflowSeconds) + delta * 5,
                                                                     Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S));
      break;
    case IDX_SAFE_TOUCH:
      settings.safeTouchC = static_cast<int16_t>(((settings.safeTouchC + 2) / 5) * 5);
      settings.safeTouchC =
          clampLocal<int16_t>(settings.safeTouchC + delta * 5, Limits::SAFE_TOUCH_MIN_C, Limits::SAFE_TOUCH_MAX_C);
      break;
    case IDX_SAFETY_CUTOFF:
      settings.safetyCutoffC = clampLocal<int16_t>(settings.safetyCutoffC + delta, Limits::SAFETY_MIN_C, Limits::SAFETY_MAX_C);
      break;
    case IDX_BUZZER:
      settings.buzzerEnabled = settings.buzzerEnabled ? 0 : 1;
      break;
    case IDX_COOLING_PROFILE: {
      int next = static_cast<int>(settings.coolingProfile) + delta;
      while (next < COOLING_PROFILE_RAPID) {
        next += 3;
      }
      while (next > COOLING_PROFILE_SILENT) {
        next -= 3;
      }
      settings.coolingProfile = static_cast<uint8_t>(next);
      break;
    }
    case IDX_KP:
      settings.kpX100 = clampLocal<int16_t>(settings.kpX100 + delta * 10, 0, 3000);
      break;
    case IDX_KI:
      settings.kiX100 = clampLocal<int16_t>(settings.kiX100 + delta, 0, 500);
      break;
    case IDX_KD:
      settings.kdX100 = clampLocal<int16_t>(settings.kdX100 + delta * 100, 0, 20000);
      break;
  }
}

void UiManager::beginSettingsDraft(const SettingsData &settings) {
  _draftSettings = settings;
  _draftActive = true;
}

void UiManager::commitSettingsDraft(SettingsStore &settings) {
  if (!_draftActive) {
    beginSettingsDraft(settings.data());
  }
  settings.editable() = _draftSettings;
  settings.save();
  beginSettingsDraft(settings.data());
}

void UiManager::keepSelectionVisible() {
  if (_settingsSelection < _settingsTop) {
    _settingsTop = _settingsSelection;
  } else if (_settingsSelection >= _settingsTop + SETTINGS_VISIBLE) {
    _settingsTop = _settingsSelection - SETTINGS_VISIBLE + 1;
  }
}

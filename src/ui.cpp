/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * ui.cpp: OLED user interface screens and settings editor
 */

#include "ui.h"

namespace {
constexpr uint8_t SETTINGS_COUNT = 14;
constexpr uint8_t SETTINGS_VISIBLE = 5;
constexpr uint8_t PROFILE_SETTINGS_COUNT = 10;
constexpr uint8_t PROFILE_SETTINGS_VISIBLE = 5;
constexpr uint16_t SCROLL_PIXEL_MS = 60;
constexpr uint8_t SCROLL_GAP_CHARS = 4;

enum SettingIndex : uint8_t {
  IDX_REFLOW_PROFILE,
  IDX_EDIT_PROFILE,
  IDX_SAFE_TOUCH,
  IDX_SAFETY_CUTOFF,
  IDX_BUZZER,
  IDX_LED_BRIGHTNESS,
  IDX_OLED_BRIGHTNESS,
  IDX_OLED_SLEEP,
  IDX_KP,
  IDX_KI,
  IDX_KD,
  IDX_RESET,
  IDX_ABOUT,
  IDX_BACK
};

enum ProfileSettingIndex : uint8_t {
  P_IDX_SLOT,
  P_IDX_NAME,
  P_IDX_PREHEAT_TEMP,
  P_IDX_PREHEAT_TIME,
  P_IDX_SOAK_TEMP,
  P_IDX_SOAK_TIME,
  P_IDX_REFLOW_TEMP,
  P_IDX_REFLOW_TIME,
  P_IDX_COOLING_PROFILE,
  P_IDX_BACK
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

#if REFLOW_DEBUG
void printSettingsSaveLine(const __FlashStringHelper *source, const SettingsData &data) {
  const ReflowProfile &profile = SettingsStore::activeProfile(data);
  Serial.print(F("EV settings source="));
  Serial.print(source);
  Serial.print(F(" profile=P"));
  Serial.print(data.selectedProfileIndex + 1);
  Serial.print(F(" name=\""));
  Serial.print(profile.name);
  Serial.print(F("\" safe="));
  Serial.print(data.safeTouchC);
  Serial.print(F(" cutoff="));
  Serial.print(data.safetyCutoffC);
  Serial.print(F(" buz="));
  Serial.print(data.buzzerLevel);
  Serial.print(F(" led="));
  Serial.print(data.ledBrightness);
  Serial.print(F(" oled="));
  Serial.print(data.oledBrightness);
  Serial.print(F(" sleep="));
  Serial.print(data.oledSleepTimeoutSeconds);
  Serial.print(F("s lock="));
  Serial.print(data.deviceControlsLocked ? F("on") : F("off"));
  Serial.print(F(" oledOff="));
  Serial.println(data.oledForcedOff ? F("on") : F("off"));
}
#endif
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
  _lastActivityMs = millis();
  applyDisplayBrightness(100);
  return true;
}

void UiManager::showStatus(const __FlashStringHelper *title, const __FlashStringHelper *line1,
                           const __FlashStringHelper *line2) {
  wakeDisplay(millis());
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println(title);
  _display.println();
  if (line1 != nullptr) {
    _display.println(line1);
  }
  if (line2 != nullptr) {
    _display.println(line2);
  }
  _display.display();
}

void UiManager::showStatus(const char *title, const char *line1, const char *line2) {
  wakeDisplay(millis());
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  if (title != nullptr) {
    _display.println(title);
  }
  _display.println();
  if (line1 != nullptr) {
    _display.println(line1);
  }
  if (line2 != nullptr) {
    _display.println(line2);
  }
  _display.display();
}

void UiManager::syncSettingsRevision(const SettingsStore &settings) {
  uint32_t revision = settings.revision();
  if (_knownSettingsRevision == 0) {
    _knownSettingsRevision = revision;
    return;
  }
  if (_knownSettingsRevision == revision) {
    return;
  }

  _knownSettingsRevision = revision;
  if (_draftActive) {
    beginSettingsDraft(settings.data());
  }
  wakeDisplay(millis());
}

void UiManager::handleInput(const InputEvent &event, SettingsStore &settings, ReflowController &reflow,
                            AlertManager &alerts, const TemperatureSample &sample) {
  if (event.rotation == 0 && !event.click) {
    return;
  }
  if (settings.data().deviceControlsLocked) {
    return;
  }
  uint32_t now = millis();
  if (_displaySleeping) {
    if (event.rotation != 0) {
      wakeDisplay(now);
    }
    return;
  }
  noteActivity(now);
  if (event.rotation != 0 || event.click) {
    alerts.beep(35);
  }

  if (reflow.isHeatingState() && event.click) {
#if REFLOW_DEBUG
    Serial.print(F("EV oled cmd=abort stage="));
    Serial.println(reflow.stateName());
#endif
    reflow.abort();
    _screen = Screen::Home;
    _homeSelection = 0;
    return;
  }

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
#if REFLOW_DEBUG
          Serial.println(F("EV oled cmd=ack"));
#endif
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
#if REFLOW_DEBUG
            const ReflowProfile &profile = SettingsStore::activeProfile(current);
            Serial.print(F("EV oled cmd=start profile=P"));
            Serial.print(current.selectedProfileIndex + 1);
            Serial.print(F(" name=\""));
            Serial.print(profile.name);
            Serial.println(F("\""));
#endif
            reflow.start(settings.data(), sample);
          }
        } else {
          beginSettingsDraft(current);
          _settingsSelection = 0;
          _settingsTop = 0;
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
        } else if (_settingsSelection == IDX_EDIT_PROFILE) {
          if (!_draftActive) {
            beginSettingsDraft(current);
          }
          _profileEditIndex = _draftSettings.selectedProfileIndex;
          _profileSelection = 0;
          _profileTop = 0;
          _screen = Screen::ProfileEditor;
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
        alerts.setBuzzerLevel(settings.data().buzzerLevel);
        alerts.setLedBrightness(settings.data().ledBrightness);
        _screen = Screen::Settings;
      }
      break;

    case Screen::ProfileEditor:
      if (event.rotation != 0) {
        int next = static_cast<int>(_profileSelection) + event.rotation;
        _profileSelection = clampLocal<int>(next, 0, PROFILE_SETTINGS_COUNT - 1);
        keepProfileSelectionVisible();
      }
      if (event.click) {
        if (_profileSelection == P_IDX_BACK) {
          commitSettingsDraft(settings);
          alerts.setBuzzerLevel(settings.data().buzzerLevel);
          alerts.setLedBrightness(settings.data().ledBrightness);
          _screen = Screen::Settings;
        } else if (_profileSelection != P_IDX_NAME) {
          _screen = Screen::ProfileEdit;
        }
      }
      break;

    case Screen::ProfileEdit:
      if (event.rotation != 0) {
        if (!_draftActive) {
          beginSettingsDraft(current);
        }
        adjustProfileSetting(_draftSettings, event.rotation, _profileSelection);
      }
      if (event.click) {
        commitSettingsDraft(settings);
        _screen = Screen::ProfileEditor;
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
          alerts.setBuzzerLevel(settings.data().buzzerLevel);
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
  const uint32_t uiNow = millis();
  const SettingsData &currentSettings = settings.data();
  const bool controlsLocked = currentSettings.deviceControlsLocked != 0;
  if (controlsLocked && !_wasControlsLocked) {
    _wasControlsLocked = true;
    _draftActive = false;
  } else if (!controlsLocked && _wasControlsLocked) {
    _wasControlsLocked = false;
    _draftActive = false;
    _screen = Screen::Home;
    _homeSelection = 0;
    _settingsSelection = 0;
    _settingsTop = 0;
    _profileSelection = 0;
    _profileTop = 0;
    _lastDrawMs = 0;
    wakeDisplay(uiNow);
  }

  if (controlsLocked && currentSettings.oledForcedOff) {
    if (!_displaySleeping || !_forcedDisplayOff) {
      _display.ssd1306_command(SSD1306_DISPLAYOFF);
      _displaySleeping = true;
      _forcedDisplayOff = true;
    }
    return;
  }
  if (_forcedDisplayOff) {
    _forcedDisplayOff = false;
    wakeDisplay(uiNow);
  }
  if (controlsLocked) {
    if (_displaySleeping) {
      wakeDisplay(uiNow);
    }
    noteActivity(uiNow);
    applyDisplayBrightness(currentSettings.oledBrightness);
    if (now - _lastDrawMs < Timing::DISPLAY_MS) {
      return;
    }
    _lastDrawMs = now;
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    drawControlsLocked();
    _display.display();
    return;
  }

  applyDisplayBrightness(currentSettings.oledBrightness);
  const bool sleepAllowed = reflow.state() == ReflowState::Idle && !reflow.cooldownLocked(currentSettings, sample, now) &&
                            !reflow.isFaultLike();
  if (!sleepAllowed) {
    if (_displaySleeping) {
      wakeDisplay(uiNow);
    }
    if (reflow.state() != ReflowState::Idle || reflow.cooldownLocked(currentSettings, sample, now) ||
        reflow.isFaultLike()) {
      noteActivity(uiNow);
    }
  } else if (!_displaySleeping) {
    uint32_t timeoutMs = static_cast<uint32_t>(currentSettings.oledSleepTimeoutSeconds) * 1000UL;
    if (timeoutMs > 0 && uiNow - _lastActivityMs >= timeoutMs) {
      if (_draftActive) {
        beginSettingsDraft(settings.data());
      }
      sleepDisplay();
      return;
    }
  }
  if (_displaySleeping) {
    return;
  }
  if (now - _lastDrawMs < Timing::DISPLAY_MS) {
    return;
  }
  _lastDrawMs = now;
  if (_screen != Screen::Home && (reflow.state() != ReflowState::Idle || reflow.cooldownLocked(currentSettings, sample, now))) {
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
    case Screen::ProfileEditor:
      drawProfileEditor(_draftActive ? _draftSettings : settings.data());
      break;
    case Screen::ProfileEdit:
      drawProfileEdit(_draftActive ? _draftSettings : settings.data());
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
  _display.print(heater.dcPwmMode() ? F("MOS:") : F("SSR:"));
  _display.print(heater.outputOn() ? F("ON ") : F("OFF"));
  _display.print(F(" "));
  _display.print(heater.dutyPercent(), 0);
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
    bool focused = index == _settingsSelection;
    char value[REFLOW_PROFILE_NAME_LEN + 12];
    settingValueText(visibleSettings, index, value, sizeof(value));
    _display.setCursor(0, y);
    _display.print(focused ? F(">") : F(" "));
    size_t valueLength = strlen(value);
    uint8_t labelWidth = 120;
    if (valueLength > 0) {
      uint8_t valueWidth = valueLength * 6;
      if (valueWidth > 52) {
        valueWidth = 52;
      }
      uint8_t valueX = OLED_WIDTH - valueWidth;
      labelWidth = valueX > 10 ? valueX - 10 : 64;
      drawTextWindow(value, valueX, y, valueWidth, focused, 1, index + 32);
    }
    drawTextWindow(settingLabel(index), 8, y, labelWidth, focused, 1, index);
  }
}

void UiManager::drawEdit(const SettingsData &settings) {
  char value[REFLOW_PROFILE_NAME_LEN + 12];
  settingValueText(settings, _settingsSelection, value, sizeof(value));
  _display.setCursor(0, 0);
  _display.print(F("Edit"));
  drawTextWindow(settingLabel(_settingsSelection), 0, 18, 128, true, 1, _settingsSelection);
  _display.setTextSize(2);
  if (_settingsSelection == IDX_REFLOW_PROFILE) {
    const ReflowProfile &profile = SettingsStore::activeProfile(settings);
    _display.setCursor(0, 34);
    _display.print(F("P"));
    _display.print(settings.selectedProfileIndex + 1);
    drawTextWindow(profile.name, 36, 34, 92, true, 2, _settingsSelection + 64);
  } else {
    drawTextWindow(value, 0, 34, 128, true, 2, _settingsSelection + 64);
  }
  _display.setTextSize(1);
  drawFooter(F("Turn=change"), F("Click=save"));
}

void UiManager::drawProfileEditor(const SettingsData &settings) {
  _display.setCursor(0, 0);
  _display.print(F("Edit Profile"));
  _display.setCursor(92, 0);
  _display.print(F("#"));
  _display.print(_profileEditIndex + 1);

  for (uint8_t row = 0; row < PROFILE_SETTINGS_VISIBLE; ++row) {
    uint8_t index = _profileTop + row;
    if (index >= PROFILE_SETTINGS_COUNT) {
      break;
    }
    uint8_t y = 12 + row * 10;
    bool focused = index == _profileSelection;
    char value[REFLOW_PROFILE_NAME_LEN + 12];
    profileSettingValueText(settings, index, value, sizeof(value));
    _display.setCursor(0, y);
    _display.print(focused ? F(">") : F(" "));
    if (index == P_IDX_SLOT || index == P_IDX_NAME) {
      drawTextWindow(value, 8, y, 120, focused, 1, index + 128);
    } else {
      drawTextWindow(profileSettingLabel(index), 8, y, 70, focused, 1, index + 96);
      drawTextWindow(value, 80, y, 48, focused, 1, index + 128);
    }
  }
}

void UiManager::drawProfileEdit(const SettingsData &settings) {
  char value[REFLOW_PROFILE_NAME_LEN + 12];
  profileSettingValueText(settings, _profileSelection, value, sizeof(value));
  _display.setCursor(0, 0);
  _display.print(F("Edit Profile"));
  _display.setCursor(0, 18);
  drawTextWindow(profileSettingLabel(_profileSelection), 0, 18, 128, true, 1, _profileSelection + 160);
  _display.setTextSize(2);
  drawTextWindow(value, 0, 34, 128, true, 2, _profileSelection + 192);
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

void UiManager::drawTextWindow(const char *text, int16_t x, int16_t y, uint8_t width, bool focused, uint8_t textSize,
                               uint8_t scrollId) {
  if (text == nullptr) {
    text = "";
  }
  uint8_t charWidth = 6 * textSize;
  uint8_t visibleChars = width / charWidth;
  if (visibleChars == 0) {
    return;
  }
  size_t textLength = strlen(text);

  if (focused && textLength > visibleChars) {
    uint8_t textHeight = 8 * textSize;
    uint16_t textPixels = textLength * charWidth;
    uint16_t gapPixels = SCROLL_GAP_CHARS * charWidth;
    uint16_t cyclePixels = textPixels + gapPixels;
    uint16_t pixelOffset = (millis() / SCROLL_PIXEL_MS) % cyclePixels;
    static GFXcanvas1 canvas(OLED_WIDTH, 16);
    canvas.fillScreen(SSD1306_BLACK);
    canvas.setTextColor(SSD1306_WHITE);
    canvas.setTextSize(textSize);

    int16_t drawX = -static_cast<int16_t>(pixelOffset);
    canvas.setCursor(drawX, 0);
    canvas.print(text);
    canvas.setCursor(drawX + textPixels + gapPixels, 0);
    canvas.print(text);
    if (pixelOffset > gapPixels) {
      canvas.setCursor(drawX + (2 * (textPixels + gapPixels)), 0);
      canvas.print(text);
    }

    for (uint8_t py = 0; py < textHeight; ++py) {
      for (uint8_t px = 0; px < width; ++px) {
        _display.drawPixel(x + px, y + py, canvas.getPixel(px, py) ? SSD1306_WHITE : SSD1306_BLACK);
      }
    }
  } else {
    char window[24];
    size_t copyLength = min(static_cast<size_t>(visibleChars), sizeof(window) - 1);
    copyLength = min(copyLength, textLength);
    if (copyLength > 0) {
      memcpy(window, text, copyLength);
    }
    window[copyLength] = '\0';
    _display.setTextSize(textSize);
    _display.setCursor(x, y);
    _display.print(window);
  }
}

const char *UiManager::settingLabel(uint8_t index) const {
  static const char *const labels[] = {
      "Reflow Profile", "Edit Reflow Profile", "Safe C", "Cutoff C", "Buzzer", "LED Bright", "OLED Bright",
      "OLED Sleep", "Kp", "Ki", "Kd", "Reset", "About", "Back"};
  return labels[index];
}

const char *UiManager::profileSettingLabel(uint8_t index) const {
  static const char *const labels[] = {
      "Profile Slot", "Profile Name", "Preheat C", "Preheat s", "Soak C", "Soak s", "Reflow C", "Reflow s",
      "Cooling", "Back"};
  return labels[index];
}

void UiManager::settingValueText(const SettingsData &settings, uint8_t index, char *buffer, size_t length) const {
  switch (index) {
    case IDX_REFLOW_PROFILE:
      snprintf(buffer, length, "P%u", static_cast<unsigned>(settings.selectedProfileIndex + 1));
      break;
    case IDX_EDIT_PROFILE:
      snprintf(buffer, length, "");
      break;
    case IDX_SAFE_TOUCH:
      snprintf(buffer, length, "%dC", settings.safeTouchC);
      break;
    case IDX_SAFETY_CUTOFF:
      snprintf(buffer, length, "%dC", settings.safetyCutoffC);
      break;
    case IDX_BUZZER:
      snprintf(buffer, length, "%u", static_cast<unsigned>(settings.buzzerLevel));
      break;
    case IDX_LED_BRIGHTNESS:
      snprintf(buffer, length, "%u%%", static_cast<unsigned>(settings.ledBrightness));
      break;
    case IDX_OLED_BRIGHTNESS:
      snprintf(buffer, length, "%u%%", static_cast<unsigned>(settings.oledBrightness));
      break;
    case IDX_OLED_SLEEP:
      snprintf(buffer, length, "%s", SettingsStore::oledSleepTimeoutLabel(settings.oledSleepTimeoutSeconds));
      break;
    case IDX_KP:
      dtostrf(SettingsStore::kp(settings), 0, 2, buffer);
      break;
    case IDX_KI:
      dtostrf(SettingsStore::ki(settings), 0, 2, buffer);
      break;
    case IDX_KD:
      dtostrf(SettingsStore::kd(settings), 0, 2, buffer);
      break;
    case IDX_RESET:
    case IDX_ABOUT:
    case IDX_BACK:
      snprintf(buffer, length, "");
      break;
    default:
      snprintf(buffer, length, "");
      break;
  }
}

void UiManager::profileSettingValueText(const SettingsData &settings, uint8_t index, char *buffer, size_t length) const {
  const ReflowProfile &profile = settings.profiles[_profileEditIndex];
  switch (index) {
    case P_IDX_SLOT:
      snprintf(buffer, length, "Profile-%u", static_cast<unsigned>(_profileEditIndex + 1));
      break;
    case P_IDX_NAME:
      snprintf(buffer, length, "%s", profile.name);
      break;
    case P_IDX_PREHEAT_TEMP:
      snprintf(buffer, length, "%dC", profile.preheatTempC);
      break;
    case P_IDX_PREHEAT_TIME:
      snprintf(buffer, length, "%us", static_cast<unsigned>(profile.preheatSeconds));
      break;
    case P_IDX_SOAK_TEMP:
      snprintf(buffer, length, "%dC", profile.soakTempC);
      break;
    case P_IDX_SOAK_TIME:
      snprintf(buffer, length, "%us", static_cast<unsigned>(profile.soakSeconds));
      break;
    case P_IDX_REFLOW_TEMP:
      snprintf(buffer, length, "%dC", profile.reflowTempC);
      break;
    case P_IDX_REFLOW_TIME:
      snprintf(buffer, length, "%us", static_cast<unsigned>(profile.reflowSeconds));
      break;
    case P_IDX_COOLING_PROFILE:
      snprintf(buffer, length, "%s", SettingsStore::coolingProfileName(profile.coolingProfile));
      break;
    case P_IDX_BACK:
      snprintf(buffer, length, ">");
      break;
    default:
      snprintf(buffer, length, "");
      break;
  }
}

void UiManager::adjustSetting(SettingsData &settings, int8_t delta, uint8_t index) {
  switch (index) {
    case IDX_REFLOW_PROFILE: {
      int next = static_cast<int>(settings.selectedProfileIndex) + delta;
      while (next < 0) {
        next += REFLOW_PROFILE_COUNT;
      }
      while (next >= REFLOW_PROFILE_COUNT) {
        next -= REFLOW_PROFILE_COUNT;
      }
      settings.selectedProfileIndex = static_cast<uint8_t>(next);
      break;
    }
    case IDX_SAFE_TOUCH:
      settings.safeTouchC = static_cast<int16_t>(((settings.safeTouchC + 2) / 5) * 5);
      settings.safeTouchC =
          clampLocal<int16_t>(settings.safeTouchC + delta * 5, Limits::SAFE_TOUCH_MIN_C, Limits::SAFE_TOUCH_MAX_C);
      break;
    case IDX_SAFETY_CUTOFF:
      settings.safetyCutoffC = clampLocal<int16_t>(settings.safetyCutoffC + delta, Limits::SAFETY_MIN_C, Limits::SAFETY_MAX_C);
      break;
    case IDX_BUZZER:
      settings.buzzerLevel = clampLocal<int>(static_cast<int>(settings.buzzerLevel) + delta, 0, 5);
      break;
    case IDX_LED_BRIGHTNESS: {
      uint8_t current = clampLocal<uint8_t>(settings.ledBrightness, 0, 100);
      current = static_cast<uint8_t>(((current + 2) / 5) * 5);
      settings.ledBrightness = clampLocal<int>(static_cast<int>(current) + delta * 5, 0, 100);
      break;
    }
    case IDX_OLED_BRIGHTNESS: {
      uint8_t current = SettingsStore::normalizeOledBrightness(settings.oledBrightness);
      settings.oledBrightness =
          SettingsStore::normalizeOledBrightness(static_cast<uint8_t>(clampLocal<int>(
              static_cast<int>(current) + delta * Limits::OLED_BRIGHTNESS_STEP_PERCENT,
              Limits::OLED_BRIGHTNESS_MIN_PERCENT, Limits::OLED_BRIGHTNESS_MAX_PERCENT)));
      break;
    }
    case IDX_OLED_SLEEP: {
      static const uint16_t options[] = {15, 30, 60, 120, 300, 600, 1800};
      uint16_t normalized = SettingsStore::normalizeOledSleepTimeoutSeconds(settings.oledSleepTimeoutSeconds);
      uint8_t currentIndex = 0;
      for (uint8_t i = 0; i < sizeof(options) / sizeof(options[0]); ++i) {
        if (options[i] == normalized) {
          currentIndex = i;
          break;
        }
      }
      int next = static_cast<int>(currentIndex) + delta;
      next = clampLocal<int>(next, 0, static_cast<int>((sizeof(options) / sizeof(options[0])) - 1));
      settings.oledSleepTimeoutSeconds = options[next];
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

void UiManager::noteActivity(uint32_t now) {
  _lastActivityMs = now;
}

void UiManager::wakeDisplay(uint32_t now) {
  if (_displaySleeping) {
    _display.ssd1306_command(SSD1306_DISPLAYON);
    _displaySleeping = false;
    _forcedDisplayOff = false;
    _lastDrawMs = 0;
#if REFLOW_DEBUG
    Serial.println(F("EV oled wake"));
#endif
  }
  noteActivity(now);
}

void UiManager::sleepDisplay() {
  if (_displaySleeping) {
    return;
  }
  if (_screen != Screen::Home && _screen != Screen::Settings) {
    _screen = Screen::Settings;
  }
  _display.ssd1306_command(SSD1306_DISPLAYOFF);
  _displaySleeping = true;
#if REFLOW_DEBUG
  Serial.println(F("EV oled sleep"));
#endif
}

void UiManager::applyDisplayBrightness(uint8_t percent) {
  uint8_t normalized = SettingsStore::normalizeOledBrightness(percent);
  if (_appliedBrightness == normalized) {
    return;
  }
  uint8_t contrast = static_cast<uint8_t>((static_cast<uint16_t>(normalized) * 255U) / 100U);
  _display.ssd1306_command(SSD1306_SETCONTRAST);
  _display.ssd1306_command(contrast);
  _appliedBrightness = normalized;
}

void UiManager::drawControlsLocked() {
  _display.setCursor(0, 0);
  _display.println(F("Device Controls"));

  _display.setCursor(0, 16);
  _display.println(F("locked by Web"));

  _display.setCursor(0, 28);
  _display.println(F("Interface."));

  _display.setCursor(0, 44);
  _display.println(F("Unlock from"));

  _display.setCursor(0, 56);
  _display.println(F("Web UI."));
}

void UiManager::adjustProfileSetting(SettingsData &settings, int8_t delta, uint8_t index) {
  if (index == P_IDX_SLOT) {
    int next = static_cast<int>(_profileEditIndex) + delta;
    while (next < 0) {
      next += REFLOW_PROFILE_COUNT;
    }
    while (next >= REFLOW_PROFILE_COUNT) {
      next -= REFLOW_PROFILE_COUNT;
    }
    _profileEditIndex = static_cast<uint8_t>(next);
    return;
  }

  ReflowProfile &profile = settings.profiles[_profileEditIndex];
  const int16_t preheatMinC = clampLocal<int16_t>(
      settings.safeTouchC + Limits::PROFILE_PREHEAT_SAFE_TOUCH_OFFSET_C, Limits::PREHEAT_MIN_C, Limits::PREHEAT_MAX_C);
  switch (index) {
    case P_IDX_PREHEAT_TEMP:
      profile.preheatTempC = clampLocal<int16_t>(profile.preheatTempC + delta, preheatMinC, Limits::PREHEAT_MAX_C);
      profile.soakTempC = clampLocal<int16_t>(profile.soakTempC, profile.preheatTempC + Limits::PROFILE_STAGE_GAP_C,
                                              Limits::SOAK_MAX_C);
      profile.reflowTempC = clampLocal<int16_t>(profile.reflowTempC, profile.soakTempC + Limits::PROFILE_STAGE_GAP_C,
                                                Limits::REFLOW_MAX_C);
      break;
    case P_IDX_PREHEAT_TIME:
      profile.preheatSeconds = static_cast<uint16_t>(clampLocal<int>(static_cast<int>(profile.preheatSeconds) + delta * 5,
                                                                     Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S));
      break;
    case P_IDX_SOAK_TEMP:
      profile.soakTempC = clampLocal<int16_t>(profile.soakTempC + delta, profile.preheatTempC + Limits::PROFILE_STAGE_GAP_C,
                                              Limits::SOAK_MAX_C);
      profile.reflowTempC = clampLocal<int16_t>(profile.reflowTempC, profile.soakTempC + Limits::PROFILE_STAGE_GAP_C,
                                                Limits::REFLOW_MAX_C);
      break;
    case P_IDX_SOAK_TIME:
      profile.soakSeconds = static_cast<uint16_t>(clampLocal<int>(static_cast<int>(profile.soakSeconds) + delta * 5,
                                                                  Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S));
      break;
    case P_IDX_REFLOW_TEMP:
      profile.reflowTempC = clampLocal<int16_t>(profile.reflowTempC + delta, profile.soakTempC + Limits::PROFILE_STAGE_GAP_C,
                                                Limits::REFLOW_MAX_C);
      break;
    case P_IDX_REFLOW_TIME:
      profile.reflowSeconds = static_cast<uint16_t>(clampLocal<int>(static_cast<int>(profile.reflowSeconds) + delta * 5,
                                                                    Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S));
      break;
    case P_IDX_COOLING_PROFILE: {
      int next = static_cast<int>(profile.coolingProfile) + delta;
      while (next < COOLING_PROFILE_RAPID) {
        next += 3;
      }
      while (next > COOLING_PROFILE_SILENT) {
        next -= 3;
      }
      profile.coolingProfile = static_cast<uint8_t>(next);
      break;
    }
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
  _knownSettingsRevision = settings.revision();
  beginSettingsDraft(settings.data());
#if REFLOW_DEBUG
  printSettingsSaveLine(F("OLED"), settings.data());
#endif
  wakeDisplay(millis());
}

void UiManager::keepSelectionVisible() {
  if (_settingsSelection < _settingsTop) {
    _settingsTop = _settingsSelection;
  } else if (_settingsSelection >= _settingsTop + SETTINGS_VISIBLE) {
    _settingsTop = _settingsSelection - SETTINGS_VISIBLE + 1;
  }
}

void UiManager::keepProfileSelectionVisible() {
  if (_profileSelection < _profileTop) {
    _profileTop = _profileSelection;
  } else if (_profileSelection >= _profileTop + PROFILE_SETTINGS_VISIBLE) {
    _profileTop = _profileSelection - PROFILE_SETTINGS_VISIBLE + 1;
  }
}

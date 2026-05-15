/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * ui.h: OLED user interface manager interface
 */

#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "alerts.h"
#include "config.h"
#include "input.h"
#include "reflow.h"
#include "settings.h"

class UiManager {
public:
  explicit UiManager(TwoWire &wire);

  bool begin();
  void showStatus(const __FlashStringHelper *title, const __FlashStringHelper *line1,
                  const __FlashStringHelper *line2 = nullptr);
  void showStatus(const char *title, const char *line1, const char *line2 = nullptr);
  void syncSettingsRevision(const SettingsStore &settings);
  void handleInput(const InputEvent &event, SettingsStore &settings, ReflowController &reflow,
                   AlertManager &alerts, const TemperatureSample &sample);
  void draw(uint32_t now, const SettingsStore &settings, const ReflowController &reflow,
            const HeaterController &heater, const FanController &fan, const TemperatureSample &sample,
            const FanController *boardFan = nullptr);

private:
  enum class Screen : uint8_t { Home, Settings, Edit, ProfileEditor, ProfileEdit, About, ResetConfirm };

  void drawHome(const SettingsData &settings, const ReflowController &reflow,
                const HeaterController &heater, const FanController &fan, const TemperatureSample &sample,
                const FanController *boardFan);
  void drawSettings(const SettingsStore &settings);
  void drawEdit(const SettingsData &settings);
  void drawProfileEditor(const SettingsData &settings);
  void drawProfileEdit(const SettingsData &settings);
  void drawAbout();
  void drawResetConfirm();
  void drawFooter(const __FlashStringHelper *left, const __FlashStringHelper *right);
  void drawTextWindow(const char *text, int16_t x, int16_t y, uint8_t width, bool focused, uint8_t textSize,
                      uint8_t scrollId);
  const char *settingLabel(uint8_t index) const;
  const char *profileSettingLabel(uint8_t index) const;
  void settingValueText(const SettingsData &settings, uint8_t index, char *buffer, size_t length) const;
  void profileSettingValueText(const SettingsData &settings, uint8_t index, char *buffer, size_t length) const;
  void adjustSetting(SettingsData &settings, int8_t delta, uint8_t index);
  void adjustProfileSetting(SettingsData &settings, int8_t delta, uint8_t index);
  void beginSettingsDraft(const SettingsData &settings);
  void commitSettingsDraft(SettingsStore &settings);
  void keepSelectionVisible();
  void keepProfileSelectionVisible();
  void noteActivity(uint32_t now);
  void wakeDisplay(uint32_t now);
  void sleepDisplay();
  void applyDisplayBrightness(uint8_t percent);
  void drawControlsLocked();

  Adafruit_SSD1306 _display;
  Screen _screen = Screen::Home;
  uint8_t _homeSelection = 0;
  uint8_t _settingsSelection = 0;
  uint8_t _settingsTop = 0;
  uint8_t _profileSelection = 0;
  uint8_t _profileTop = 0;
  uint8_t _profileEditIndex = 0;
  SettingsData _draftSettings{};
  bool _draftActive = false;
  bool _displaySleeping = false;
  bool _forcedDisplayOff = false;
  bool _wasControlsLocked = false;
  uint8_t _appliedBrightness = 0;
  uint32_t _knownSettingsRevision = 0;
  uint32_t _lastDrawMs = 0;
  uint32_t _lastActivityMs = 0;
};

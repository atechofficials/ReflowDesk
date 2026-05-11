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
  void handleInput(const InputEvent &event, SettingsStore &settings, ReflowController &reflow,
                   AlertManager &alerts, const TemperatureSample &sample);
  void draw(uint32_t now, const SettingsStore &settings, const ReflowController &reflow,
            const HeaterController &heater, const FanController &fan, const TemperatureSample &sample,
            const FanController *boardFan = nullptr);

private:
  enum class Screen : uint8_t { Home, Settings, Edit, About, ResetConfirm };

  void drawHome(const SettingsData &settings, const ReflowController &reflow,
                const HeaterController &heater, const FanController &fan, const TemperatureSample &sample,
                const FanController *boardFan);
  void drawSettings(const SettingsStore &settings);
  void drawEdit(const SettingsData &settings);
  void drawAbout();
  void drawResetConfirm();
  void drawFooter(const __FlashStringHelper *left, const __FlashStringHelper *right);
  const char *settingLabel(uint8_t index) const;
  void printSettingValue(const SettingsData &settings, uint8_t index);
  void adjustSetting(SettingsData &settings, int8_t delta, uint8_t index);
  void beginSettingsDraft(const SettingsData &settings);
  void commitSettingsDraft(SettingsStore &settings);
  void keepSelectionVisible();

  Adafruit_SSD1306 _display;
  Screen _screen = Screen::Home;
  uint8_t _homeSelection = 0;
  uint8_t _settingsSelection = 0;
  uint8_t _settingsTop = 0;
  SettingsData _draftSettings{};
  bool _draftActive = false;
  uint32_t _lastDrawMs = 0;
};

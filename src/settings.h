/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * settings.h: Persistent settings data model and interface
 */

#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include "config.h"

enum CoolingProfile : uint8_t {
  COOLING_PROFILE_RAPID = 0,
  COOLING_PROFILE_NORMAL = 1,
  COOLING_PROFILE_SILENT = 2,
};

struct SettingsData {
  uint32_t magic;
  uint16_t version;
  uint16_t length;
  int16_t preheatTempC;
  uint16_t preheatSeconds;
  int16_t soakTempC;
  uint16_t soakSeconds;
  int16_t reflowTempC;
  uint16_t reflowSeconds;
  int16_t safeTouchC;
  int16_t safetyCutoffC;
  uint8_t buzzerEnabled;
  uint8_t coolingProfile;
  int16_t kpX100;
  int16_t kiX100;
  int16_t kdX100;
  uint8_t ledBrightness;
  uint8_t reserved1;
  uint16_t crc;
} __attribute__((packed));

class SettingsStore {
public:
  explicit SettingsStore(TwoWire &wire);

  bool begin();
  const SettingsData &data() const { return _data; }
  SettingsData &editable() { return _data; }
  void resetDefaults();
  void validate();
  bool save();
  bool isStorageReady() const { return _storageReady; }
  bool isEepromReady() const { return isStorageReady(); }
  uint8_t eepromAddress() const { return 0; }
  const char *storageName() const { return "NVS"; }

  static SettingsData defaults();
  static float kp(const SettingsData &settings) { return settings.kpX100 / 100.0f; }
  static float ki(const SettingsData &settings) { return settings.kiX100 / 100.0f; }
  static float kd(const SettingsData &settings) { return settings.kdX100 / 100.0f; }
  static const char *coolingProfileName(uint8_t profile);

private:
  bool load();
  static uint16_t crc16(const uint8_t *data, size_t length);
  static uint16_t expectedCrc(SettingsData data);

  Preferences _prefs;
  SettingsData _data;
  bool _storageReady = false;
};

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

constexpr uint8_t REFLOW_PROFILE_COUNT = 4;
constexpr uint8_t REFLOW_PROFILE_NAME_LEN = 64;

struct ReflowProfile {
  char name[REFLOW_PROFILE_NAME_LEN];
  int16_t preheatTempC;
  uint16_t preheatSeconds;
  int16_t soakTempC;
  uint16_t soakSeconds;
  int16_t reflowTempC;
  uint16_t reflowSeconds;
  uint8_t coolingProfile;
  uint8_t reserved1;
  uint16_t reserved2;
  uint32_t jsonCrc;
} __attribute__((packed));

struct SettingsData {
  uint32_t magic;
  uint16_t version;
  uint16_t length;
  uint8_t selectedProfileIndex;
  uint8_t buzzerLevel;
  uint8_t ledBrightness;
  uint8_t reserved1;
  ReflowProfile profiles[REFLOW_PROFILE_COUNT];
  int16_t safeTouchC;
  int16_t safetyCutoffC;
  int16_t kpX100;
  int16_t kiX100;
  int16_t kdX100;
  uint16_t oledSleepTimeoutSeconds;
  uint8_t oledBrightness;
  uint8_t deviceControlsLocked;
  uint8_t oledForcedOff;
  uint8_t reserved2;
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
  bool importProfilesFromLittleFs();
  uint32_t revision() const { return _revision; }
  uint32_t resetCount() const { return _resetCount; }
  bool isStorageReady() const { return _storageReady; }
  bool isEepromReady() const { return isStorageReady(); }
  uint8_t eepromAddress() const { return 0; }
  const char *storageName() const { return "NVS"; }

  static SettingsData defaults();
  static ReflowProfile defaultProfile(uint8_t index);
  static const ReflowProfile &activeProfile(const SettingsData &settings);
  static ReflowProfile &activeProfile(SettingsData &settings);
  static float kp(const SettingsData &settings) { return settings.kpX100 / 100.0f; }
  static float ki(const SettingsData &settings) { return settings.kiX100 / 100.0f; }
  static float kd(const SettingsData &settings) { return settings.kdX100 / 100.0f; }
  static const char *coolingProfileName(uint8_t profile);
  static uint16_t normalizeOledSleepTimeoutSeconds(uint16_t seconds);
  static uint8_t normalizeOledBrightness(uint8_t percent);
  static const char *oledSleepTimeoutLabel(uint16_t seconds);

private:
  bool load();
  bool loadLegacy();
  bool importProfileFromLittleFs(uint8_t index);
  static uint16_t crc16(const uint8_t *data, size_t length);
  static uint16_t expectedCrc(SettingsData data);
  static uint32_t fnv1a32(const uint8_t *data, size_t length);
  static void copyProfileName(ReflowProfile &profile, const char *name);
  static void validateProfile(ReflowProfile &profile, uint8_t index, int16_t preheatMinC = Limits::PREHEAT_MIN_C);
  static uint8_t coolingProfileFromName(const char *name, uint8_t fallback);

  Preferences _prefs;
  SettingsData _data;
  uint32_t _revision = 1;
  uint32_t _resetCount = 0;
  bool _storageReady = false;
};

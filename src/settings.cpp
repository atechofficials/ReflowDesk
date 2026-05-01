/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * settings.cpp: Persistent settings storage and validation
 */

#include "settings.h"

#include <stddef.h>
#include <string.h>

namespace {
constexpr uint32_t SETTINGS_MAGIC = 0x52465031UL; // RFP1
constexpr uint16_t SETTINGS_VERSION = 2;
constexpr uint16_t SETTINGS_LEGACY_VERSION = 1;
constexpr const char *SETTINGS_NAMESPACE = "reflow";
constexpr const char *SETTINGS_KEY = "settings";

template <typename T>
T clampValue(T value, T low, T high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}
}

SettingsStore::SettingsStore(TwoWire &wire)
    : _data(defaults()) {
  (void)wire;
}

bool SettingsStore::begin() {
  _storageReady = _prefs.begin(SETTINGS_NAMESPACE, false);

#if REFLOW_DEBUG
  Serial.print(F("NVS begin ns="));
  Serial.print(SETTINGS_NAMESPACE);
  Serial.print(F(" ready="));
  Serial.println(_storageReady ? F("yes") : F("no"));
#endif

  if (!_storageReady) {
    _data = defaults();
#if REFLOW_DEBUG
    Serial.println(F("NVS offline; using RAM defaults only"));
#endif
    return false;
  }

  if (!load()) {
    _data = defaults();
#if REFLOW_DEBUG
    Serial.println(F("NVS load failed; using defaults until settings are saved"));
#endif
  }
  return _storageReady;
}

SettingsData SettingsStore::defaults() {
  SettingsData settings{};
  settings.magic = SETTINGS_MAGIC;
  settings.version = SETTINGS_VERSION;
  settings.length = sizeof(SettingsData);
  settings.preheatTempC = 150;
  settings.preheatSeconds = 120;
  settings.soakTempC = 180;
  settings.soakSeconds = 60;
  settings.reflowTempC = 200;
  settings.reflowSeconds = 30;
  settings.safeTouchC = 45;
  settings.safetyCutoffC = 235;
  settings.buzzerEnabled = 1;
  settings.coolingProfile = COOLING_PROFILE_NORMAL;
  settings.kpX100 = 700;
  settings.kiX100 = 6;
  settings.kdX100 = 2000;
  settings.ledBrightness = 45;
  settings.crc = expectedCrc(settings);
  return settings;
}

void SettingsStore::resetDefaults() {
  _data = defaults();
}

void SettingsStore::validate() {
  _data.magic = SETTINGS_MAGIC;
  _data.version = SETTINGS_VERSION;
  _data.length = sizeof(SettingsData);
  _data.preheatTempC = clampValue<int16_t>(_data.preheatTempC, Limits::PREHEAT_MIN_C, Limits::PREHEAT_MAX_C);
  _data.soakTempC = clampValue<int16_t>(_data.soakTempC, Limits::SOAK_MIN_C, Limits::SOAK_MAX_C);
  _data.reflowTempC = clampValue<int16_t>(_data.reflowTempC, Limits::REFLOW_MIN_C, Limits::REFLOW_MAX_C);
  _data.preheatSeconds = clampValue<uint16_t>(_data.preheatSeconds, Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  _data.soakSeconds = clampValue<uint16_t>(_data.soakSeconds, Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  _data.reflowSeconds = clampValue<uint16_t>(_data.reflowSeconds, Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  _data.safeTouchC = clampValue<int16_t>(_data.safeTouchC, Limits::SAFE_TOUCH_MIN_C, Limits::SAFE_TOUCH_MAX_C);
  _data.safeTouchC = static_cast<int16_t>(((_data.safeTouchC + 2) / 5) * 5);
  _data.safeTouchC = clampValue<int16_t>(_data.safeTouchC, Limits::SAFE_TOUCH_MIN_C, Limits::SAFE_TOUCH_MAX_C);
  _data.safetyCutoffC = clampValue<int16_t>(_data.safetyCutoffC, Limits::SAFETY_MIN_C, Limits::SAFETY_MAX_C);
  if (_data.safetyCutoffC <= _data.reflowTempC) {
    _data.safetyCutoffC = clampValue<int16_t>(_data.reflowTempC + 15, Limits::SAFETY_MIN_C, Limits::SAFETY_MAX_C);
  }
  _data.buzzerEnabled = _data.buzzerEnabled ? 1 : 0;
  if (_data.coolingProfile > COOLING_PROFILE_SILENT) {
    _data.coolingProfile = COOLING_PROFILE_NORMAL;
  }
  _data.kpX100 = clampValue<int16_t>(_data.kpX100, 0, 3000);
  _data.kiX100 = clampValue<int16_t>(_data.kiX100, 0, 500);
  _data.kdX100 = clampValue<int16_t>(_data.kdX100, 0, 20000);
  _data.ledBrightness = clampValue<uint8_t>(_data.ledBrightness, 5, 120);
  _data.crc = expectedCrc(_data);
}

bool SettingsStore::load() {
  SettingsData candidate{};
  size_t read = _prefs.getBytes(SETTINGS_KEY, &candidate, sizeof(candidate));
#if REFLOW_DEBUG
  Serial.print(F("NVS load read="));
  Serial.print(read);
  Serial.print(F("/"));
  Serial.println(sizeof(candidate));
#endif
  if (read != sizeof(candidate)) {
#if REFLOW_DEBUG
    Serial.println(F("NVS load reject: read length"));
#endif
    return false;
  }
  bool legacyVersion = candidate.version == SETTINGS_LEGACY_VERSION;
  bool supportedVersion = candidate.version == SETTINGS_VERSION || legacyVersion;
  if (candidate.magic != SETTINGS_MAGIC || !supportedVersion || candidate.length != sizeof(SettingsData)) {
#if REFLOW_DEBUG
    Serial.print(F("NVS load reject: hdr magic=0x"));
    Serial.print(candidate.magic, HEX);
    Serial.print(F(" ver="));
    Serial.print(candidate.version);
    Serial.print(F(" len="));
    Serial.print(candidate.length);
    Serial.print(F(" expected_len="));
    Serial.println(sizeof(SettingsData));
#endif
    return false;
  }
  uint16_t expected = expectedCrc(candidate);
  if (candidate.crc != expected) {
#if REFLOW_DEBUG
    Serial.print(F("NVS load reject: crc stored=0x"));
    Serial.print(candidate.crc, HEX);
    Serial.print(F(" expected=0x"));
    Serial.println(expected, HEX);
#endif
    return false;
  }
  if (legacyVersion) {
    candidate.version = SETTINGS_VERSION;
    candidate.coolingProfile = COOLING_PROFILE_NORMAL;
  }
  _data = candidate;
  validate();
#if REFLOW_DEBUG
  Serial.print(F("NVS load ok crc=0x"));
  Serial.print(_data.crc, HEX);
  Serial.print(F(" safe="));
  Serial.print(_data.safeTouchC);
  Serial.print(F(" cool="));
  Serial.print(coolingProfileName(_data.coolingProfile));
  Serial.print(F(" pid="));
  Serial.print(kp(_data), 2);
  Serial.print(F(","));
  Serial.print(ki(_data), 2);
  Serial.print(F(","));
  Serial.println(kd(_data), 2);
#endif
  return true;
}

bool SettingsStore::save() {
  validate();
  if (!_storageReady) {
#if REFLOW_DEBUG
    Serial.println(F("NVS save fail: storage offline"));
#endif
    return false;
  }

  SettingsData stored{};
  size_t read = _prefs.getBytes(SETTINGS_KEY, &stored, sizeof(stored));
  const uint8_t *desiredBytes = reinterpret_cast<const uint8_t *>(&_data);
  const uint8_t *storedBytes = reinterpret_cast<const uint8_t *>(&stored);

  if (read == sizeof(stored) && memcmp(desiredBytes, storedBytes, sizeof(_data)) == 0) {
#if REFLOW_DEBUG
    Serial.print(F("NVS save skip: unchanged crc=0x"));
    Serial.println(_data.crc, HEX);
#endif
    return true;
  }

  size_t written = _prefs.putBytes(SETTINGS_KEY, desiredBytes, sizeof(_data));
  SettingsData verify{};
  size_t verifyRead = _prefs.getBytes(SETTINGS_KEY, &verify, sizeof(verify));
  bool verified = written == sizeof(_data) && verifyRead == sizeof(verify) &&
                  memcmp(desiredBytes, reinterpret_cast<const uint8_t *>(&verify), sizeof(_data)) == 0;
#if REFLOW_DEBUG
  Serial.print(F("NVS save "));
  Serial.print(verified ? F("ok") : F("fail"));
  Serial.print(F(" read="));
  Serial.print(read);
  Serial.print(F(" written="));
  Serial.print(written);
  Serial.print(F(" crc=0x"));
  Serial.println(_data.crc, HEX);
#endif
  return verified;
}

uint16_t SettingsStore::crc16(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021) : static_cast<uint16_t>(crc << 1);
    }
  }
  return crc;
}

uint16_t SettingsStore::expectedCrc(SettingsData data) {
  data.crc = 0;
  return crc16(reinterpret_cast<const uint8_t *>(&data), offsetof(SettingsData, crc));
}

const char *SettingsStore::coolingProfileName(uint8_t profile) {
  switch (profile) {
    case COOLING_PROFILE_RAPID:
      return "Rapid";
    case COOLING_PROFILE_SILENT:
      return "Silent";
    case COOLING_PROFILE_NORMAL:
    default:
      return "Normal";
  }
}

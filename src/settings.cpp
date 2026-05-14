/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * settings.cpp: Persistent settings storage and validation
 */

#include "settings.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <stddef.h>
#include <strings.h>
#include <string.h>

namespace {
constexpr uint32_t SETTINGS_MAGIC = 0x52465031UL; // RFP1
constexpr uint16_t SETTINGS_VERSION = 5;
constexpr uint16_t SETTINGS_LEGACY_VERSION_4 = 4;
constexpr uint16_t SETTINGS_LEGACY_VERSION_3 = 3;
constexpr uint16_t SETTINGS_LEGACY_VERSION_2 = 2;
constexpr uint16_t SETTINGS_LEGACY_VERSION_1 = 1;
constexpr const char *SETTINGS_NAMESPACE = "reflow";
constexpr const char *SETTINGS_KEY = "settings";
constexpr uint8_t JSON_SCHEMA_VERSION = 1;

struct LegacySettingsData {
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

template <typename T>
T clampValue(T value, T low, T high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

void buildProfilePath(uint8_t index, char *path, size_t length) {
  snprintf(path, length, "/profiles/profile-%u.json", static_cast<unsigned>(index + 1));
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
  if (importProfilesFromLittleFs()) {
    save();
  }
  return _storageReady;
}

void SettingsStore::copyProfileName(ReflowProfile &profile, const char *name) {
  if (name == nullptr || name[0] == '\0') {
    return;
  }
  strncpy(profile.name, name, sizeof(profile.name) - 1);
  profile.name[sizeof(profile.name) - 1] = '\0';
}

ReflowProfile SettingsStore::defaultProfile(uint8_t index) {
  ReflowProfile profile{};
  switch (index) {
    case 0:
      copyProfileName(profile, "Sn63Pb37 Leaded");
      profile.preheatTempC = 150;
      profile.preheatSeconds = 90;
      profile.soakTempC = 180;
      profile.soakSeconds = 90;
      profile.reflowTempC = 215;
      profile.reflowSeconds = 45;
      profile.coolingProfile = COOLING_PROFILE_NORMAL;
      break;
    case 1:
      copyProfileName(profile, "SAC305 Lead-Free");
      profile.preheatTempC = 150;
      profile.preheatSeconds = 120;
      profile.soakTempC = 180;
      profile.soakSeconds = 90;
      profile.reflowTempC = 235;
      profile.reflowSeconds = 45;
      profile.coolingProfile = COOLING_PROFILE_RAPID;
      break;
    case 2:
      copyProfileName(profile, "Low Temp SnBi");
      profile.preheatTempC = 120;
      profile.preheatSeconds = 90;
      profile.soakTempC = 150;
      profile.soakSeconds = 90;
      profile.reflowTempC = 180;
      profile.reflowSeconds = 45;
      profile.coolingProfile = COOLING_PROFILE_SILENT;
      break;
    case 3:
    default:
      copyProfileName(profile, "Generic Conservative");
      profile.preheatTempC = 150;
      profile.preheatSeconds = 120;
      profile.soakTempC = 180;
      profile.soakSeconds = 60;
      profile.reflowTempC = 200;
      profile.reflowSeconds = 30;
      profile.coolingProfile = COOLING_PROFILE_NORMAL;
      break;
  }
  validateProfile(profile, index);
  return profile;
}

SettingsData SettingsStore::defaults() {
  SettingsData settings{};
  settings.magic = SETTINGS_MAGIC;
  settings.version = SETTINGS_VERSION;
  settings.length = sizeof(SettingsData);
  settings.selectedProfileIndex = 0;
  for (uint8_t i = 0; i < REFLOW_PROFILE_COUNT; ++i) {
    settings.profiles[i] = defaultProfile(i);
  }
  settings.safeTouchC = 45;
  settings.safetyCutoffC = 245;
  settings.buzzerLevel = 3;
  settings.kpX100 = 700;
  settings.kiX100 = 6;
  settings.kdX100 = 2000;
  settings.ledBrightness = 45;
  settings.oledSleepTimeoutSeconds = normalizeOledSleepTimeoutSeconds(Timing::OLED_SLEEP_DEFAULT_SECONDS);
  settings.crc = expectedCrc(settings);
  return settings;
}

void SettingsStore::resetDefaults() {
  _data = defaults();
  importProfilesFromLittleFs();
  ++_resetCount;
}

void SettingsStore::validate() {
  _data.magic = SETTINGS_MAGIC;
  _data.version = SETTINGS_VERSION;
  _data.length = sizeof(SettingsData);
  if (_data.selectedProfileIndex >= REFLOW_PROFILE_COUNT) {
    _data.selectedProfileIndex = 0;
  }
  _data.safeTouchC = clampValue<int16_t>(_data.safeTouchC, Limits::SAFE_TOUCH_MIN_C, Limits::SAFE_TOUCH_MAX_C);
  _data.safeTouchC = static_cast<int16_t>(((_data.safeTouchC + 2) / 5) * 5);
  _data.safeTouchC = clampValue<int16_t>(_data.safeTouchC, Limits::SAFE_TOUCH_MIN_C, Limits::SAFE_TOUCH_MAX_C);
  const int16_t preheatMinC = clampValue<int16_t>(
      _data.safeTouchC + Limits::PROFILE_PREHEAT_SAFE_TOUCH_OFFSET_C, Limits::PREHEAT_MIN_C, Limits::PREHEAT_MAX_C);
  int16_t highestReflowC = 0;
  for (uint8_t i = 0; i < REFLOW_PROFILE_COUNT; ++i) {
    validateProfile(_data.profiles[i], i, preheatMinC);
    if (_data.profiles[i].reflowTempC > highestReflowC) {
      highestReflowC = _data.profiles[i].reflowTempC;
    }
  }
  _data.safetyCutoffC = clampValue<int16_t>(_data.safetyCutoffC, Limits::SAFETY_MIN_C, Limits::SAFETY_MAX_C);
  if (_data.safetyCutoffC <= highestReflowC) {
    _data.safetyCutoffC = clampValue<int16_t>(highestReflowC + 10, Limits::SAFETY_MIN_C, Limits::SAFETY_MAX_C);
  }
  _data.buzzerLevel = clampValue<uint8_t>(_data.buzzerLevel, 0, 5);
  _data.kpX100 = clampValue<int16_t>(_data.kpX100, 0, 3000);
  _data.kiX100 = clampValue<int16_t>(_data.kiX100, 0, 500);
  _data.kdX100 = clampValue<int16_t>(_data.kdX100, 0, 20000);
  _data.ledBrightness = clampValue<uint8_t>(_data.ledBrightness, 0, 100);
  _data.ledBrightness = static_cast<uint8_t>(((_data.ledBrightness + 2) / 5) * 5);
  _data.ledBrightness = clampValue<uint8_t>(_data.ledBrightness, 0, 100);
  _data.oledSleepTimeoutSeconds = normalizeOledSleepTimeoutSeconds(_data.oledSleepTimeoutSeconds);
  _data.crc = expectedCrc(_data);
}

bool SettingsStore::load() {
  size_t storedLength = _prefs.getBytesLength(SETTINGS_KEY);
#if REFLOW_DEBUG
  Serial.print(F("NVS stored len="));
  Serial.println(storedLength);
#endif
  if (storedLength == sizeof(LegacySettingsData)) {
    return loadLegacy();
  }

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
  if (candidate.magic == SETTINGS_MAGIC &&
      (candidate.version == SETTINGS_LEGACY_VERSION_4 || candidate.version == SETTINGS_LEGACY_VERSION_3) &&
      candidate.length == sizeof(SettingsData)) {
    uint16_t expected = expectedCrc(candidate);
    if (candidate.crc != expected) {
#if REFLOW_DEBUG
      Serial.print(F("NVS v3 load reject: crc stored=0x"));
      Serial.print(candidate.crc, HEX);
      Serial.print(F(" expected=0x"));
      Serial.println(expected, HEX);
#endif
      return false;
    }
    _data = candidate;
    if (candidate.version == SETTINGS_LEGACY_VERSION_3) {
      _data.buzzerLevel = _data.buzzerLevel ? 3 : 0;
    }
    if (_data.oledSleepTimeoutSeconds == 0) {
      _data.oledSleepTimeoutSeconds = normalizeOledSleepTimeoutSeconds(Timing::OLED_SLEEP_DEFAULT_SECONDS);
    }
    validate();
#if REFLOW_DEBUG
    Serial.println(candidate.version == SETTINGS_LEGACY_VERSION_4
                       ? F("NVS v4 settings migrated to OLED sleep timeout")
                       : F("NVS v3 settings migrated to buzzer level and OLED sleep timeout"));
#endif
    return true;
  }
  if (candidate.magic != SETTINGS_MAGIC || candidate.version != SETTINGS_VERSION || candidate.length != sizeof(SettingsData)) {
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
  _data = candidate;
  validate();
#if REFLOW_DEBUG
  Serial.print(F("NVS load ok crc=0x"));
  Serial.print(_data.crc, HEX);
  Serial.print(F(" safe="));
  Serial.print(_data.safeTouchC);
  Serial.print(F(" profile="));
  Serial.print(_data.selectedProfileIndex + 1);
  Serial.print(F(" cool="));
  Serial.print(coolingProfileName(activeProfile(_data).coolingProfile));
  Serial.print(F(" pid="));
  Serial.print(kp(_data), 2);
  Serial.print(F(","));
  Serial.print(ki(_data), 2);
  Serial.print(F(","));
  Serial.println(kd(_data), 2);
#endif
  return true;
}

bool SettingsStore::loadLegacy() {
  LegacySettingsData legacy{};
  size_t read = _prefs.getBytes(SETTINGS_KEY, &legacy, sizeof(legacy));
  if (read != sizeof(legacy)) {
    return false;
  }
  bool supportedVersion = legacy.version == SETTINGS_LEGACY_VERSION_2 || legacy.version == SETTINGS_LEGACY_VERSION_1;
  if (legacy.magic != SETTINGS_MAGIC || !supportedVersion || legacy.length != sizeof(LegacySettingsData)) {
    return false;
  }
  uint16_t storedCrc = legacy.crc;
  legacy.crc = 0;
  uint16_t expected = crc16(reinterpret_cast<const uint8_t *>(&legacy), offsetof(LegacySettingsData, crc));
  legacy.crc = storedCrc;
  if (storedCrc != expected) {
    return false;
  }

  _data = defaults();
  ReflowProfile migrated = _data.profiles[0];
  copyProfileName(migrated, "Profile-1");
  migrated.preheatTempC = legacy.preheatTempC;
  migrated.preheatSeconds = legacy.preheatSeconds;
  migrated.soakTempC = legacy.soakTempC;
  migrated.soakSeconds = legacy.soakSeconds;
  migrated.reflowTempC = legacy.reflowTempC;
  migrated.reflowSeconds = legacy.reflowSeconds;
  migrated.coolingProfile = legacy.version == SETTINGS_LEGACY_VERSION_1 ? COOLING_PROFILE_NORMAL : legacy.coolingProfile;
  migrated.jsonCrc = 0;
  validateProfile(migrated, 0);
  _data.profiles[0] = migrated;
  _data.selectedProfileIndex = 0;
  _data.safeTouchC = legacy.safeTouchC;
  _data.safetyCutoffC = legacy.safetyCutoffC;
  _data.buzzerLevel = legacy.buzzerEnabled ? 3 : 0;
  _data.kpX100 = legacy.kpX100;
  _data.kiX100 = legacy.kiX100;
  _data.kdX100 = legacy.kdX100;
  _data.ledBrightness = legacy.ledBrightness;
  _data.oledSleepTimeoutSeconds = normalizeOledSleepTimeoutSeconds(Timing::OLED_SLEEP_DEFAULT_SECONDS);
  validate();
#if REFLOW_DEBUG
  Serial.println(F("NVS legacy settings migrated to profile 1"));
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
  if (verified) {
    ++_revision;
  }
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

uint32_t SettingsStore::fnv1a32(const uint8_t *data, size_t length) {
  uint32_t hash = 2166136261UL;
  for (size_t i = 0; i < length; ++i) {
    hash ^= data[i];
    hash *= 16777619UL;
  }
  return hash;
}

void SettingsStore::validateProfile(ReflowProfile &profile, uint8_t index, int16_t preheatMinC) {
  if (profile.name[0] == '\0') {
    char fallback[16];
    snprintf(fallback, sizeof(fallback), "Profile-%u", static_cast<unsigned>(index + 1));
    copyProfileName(profile, fallback);
  }
  profile.name[sizeof(profile.name) - 1] = '\0';
  preheatMinC = clampValue<int16_t>(preheatMinC, Limits::PREHEAT_MIN_C, Limits::PREHEAT_MAX_C);
  profile.preheatTempC = clampValue<int16_t>(profile.preheatTempC, preheatMinC, Limits::PREHEAT_MAX_C);
  const int16_t soakMinC = clampValue<int16_t>(
      profile.preheatTempC + Limits::PROFILE_STAGE_GAP_C, Limits::PREHEAT_MIN_C, Limits::SOAK_MAX_C);
  profile.soakTempC = clampValue<int16_t>(profile.soakTempC, soakMinC, Limits::SOAK_MAX_C);
  const int16_t reflowMinC = clampValue<int16_t>(
      profile.soakTempC + Limits::PROFILE_STAGE_GAP_C, Limits::PREHEAT_MIN_C, Limits::REFLOW_MAX_C);
  profile.reflowTempC = clampValue<int16_t>(profile.reflowTempC, reflowMinC, Limits::REFLOW_MAX_C);
  profile.preheatSeconds = clampValue<uint16_t>(profile.preheatSeconds, Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  profile.soakSeconds = clampValue<uint16_t>(profile.soakSeconds, Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  profile.reflowSeconds = clampValue<uint16_t>(profile.reflowSeconds, Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  if (profile.coolingProfile > COOLING_PROFILE_SILENT) {
    profile.coolingProfile = COOLING_PROFILE_NORMAL;
  }
}

const ReflowProfile &SettingsStore::activeProfile(const SettingsData &settings) {
  uint8_t index = settings.selectedProfileIndex;
  if (index >= REFLOW_PROFILE_COUNT) {
    index = 0;
  }
  return settings.profiles[index];
}

ReflowProfile &SettingsStore::activeProfile(SettingsData &settings) {
  uint8_t index = settings.selectedProfileIndex;
  if (index >= REFLOW_PROFILE_COUNT) {
    index = 0;
  }
  return settings.profiles[index];
}

uint8_t SettingsStore::coolingProfileFromName(const char *name, uint8_t fallback) {
  if (name == nullptr) {
    return fallback;
  }
  if (strcasecmp(name, "rapid") == 0) {
    return COOLING_PROFILE_RAPID;
  }
  if (strcasecmp(name, "normal") == 0) {
    return COOLING_PROFILE_NORMAL;
  }
  if (strcasecmp(name, "silent") == 0) {
    return COOLING_PROFILE_SILENT;
  }
  return fallback;
}

bool SettingsStore::importProfilesFromLittleFs() {
  if (!LittleFS.begin(false)) {
#if REFLOW_DEBUG
    Serial.println(F("LittleFS mount failed; profile JSON import skipped"));
#endif
    return false;
  }

  bool changed = false;
  for (uint8_t i = 0; i < REFLOW_PROFILE_COUNT; ++i) {
    changed = importProfileFromLittleFs(i) || changed;
  }
  if (changed) {
    validate();
  }
  return changed;
}

bool SettingsStore::importProfileFromLittleFs(uint8_t index) {
  char path[32];
  buildProfilePath(index, path, sizeof(path));
  File file = LittleFS.open(path, "r");
  if (!file) {
#if REFLOW_DEBUG
    Serial.print(F("Profile JSON missing: "));
    Serial.println(path);
#endif
    return false;
  }

  String json = file.readString();
  uint32_t jsonCrc = fnv1a32(reinterpret_cast<const uint8_t *>(json.c_str()), json.length());
  if (_data.profiles[index].jsonCrc == jsonCrc) {
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
#if REFLOW_DEBUG
    Serial.print(F("Profile JSON parse fail: "));
    Serial.println(path);
#endif
    return false;
  }

  uint8_t schema = doc["schema"] | JSON_SCHEMA_VERSION;
  uint8_t slot = doc["slot"] | static_cast<uint8_t>(index + 1);
  if (schema != JSON_SCHEMA_VERSION || slot != index + 1) {
#if REFLOW_DEBUG
    Serial.print(F("Profile JSON reject: "));
    Serial.println(path);
#endif
    return false;
  }

  ReflowProfile profile = _data.profiles[index];
  const char *name = doc["name"] | profile.name;
  copyProfileName(profile, name);
  profile.preheatTempC = doc["preheat"]["tempC"] | profile.preheatTempC;
  profile.preheatSeconds = doc["preheat"]["seconds"] | profile.preheatSeconds;
  profile.soakTempC = doc["soak"]["tempC"] | profile.soakTempC;
  profile.soakSeconds = doc["soak"]["seconds"] | profile.soakSeconds;
  profile.reflowTempC = doc["reflow"]["tempC"] | profile.reflowTempC;
  profile.reflowSeconds = doc["reflow"]["seconds"] | profile.reflowSeconds;
  if (doc["coolingProfile"].is<const char *>()) {
    profile.coolingProfile = coolingProfileFromName(doc["coolingProfile"], profile.coolingProfile);
  } else {
    profile.coolingProfile = doc["coolingProfile"] | profile.coolingProfile;
  }
  profile.jsonCrc = jsonCrc;
  const int16_t preheatMinC = clampValue<int16_t>(
      _data.safeTouchC + Limits::PROFILE_PREHEAT_SAFE_TOUCH_OFFSET_C, Limits::PREHEAT_MIN_C, Limits::PREHEAT_MAX_C);
  validateProfile(profile, index, preheatMinC);
  _data.profiles[index] = profile;
#if REFLOW_DEBUG
  Serial.print(F("Profile JSON imported: "));
  Serial.println(path);
#endif
  return true;
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

uint16_t SettingsStore::normalizeOledSleepTimeoutSeconds(uint16_t seconds) {
  static const uint16_t options[] = {15, 30, 60, 120, 300, 600, 1800};
  uint16_t best = options[0];
  uint16_t bestDistance = seconds > best ? seconds - best : best - seconds;
  for (uint8_t i = 1; i < sizeof(options) / sizeof(options[0]); ++i) {
    uint16_t distance = seconds > options[i] ? seconds - options[i] : options[i] - seconds;
    if (distance < bestDistance) {
      best = options[i];
      bestDistance = distance;
    }
  }
  return best;
}

const char *SettingsStore::oledSleepTimeoutLabel(uint16_t seconds) {
  switch (normalizeOledSleepTimeoutSeconds(seconds)) {
    case 15:
      return "15s";
    case 30:
      return "30s";
    case 60:
      return "1m";
    case 120:
      return "2m";
    case 300:
      return "5m";
    case 600:
      return "10m";
    case 1800:
      return "30m";
    default:
      return "2m";
  }
}

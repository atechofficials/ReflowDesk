/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * web_server.cpp: ESP32-S3 hosted web interface
 */

#include "web_server.h"

#if REFLOW_WEB_ENABLED

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <esp_mac.h>
#include <esp_system.h>
#include <mbedtls/sha256.h>
#include <string.h>

namespace {
constexpr uint16_t HTTP_PORT = 80;
constexpr uint16_t WS_PORT = 81;
constexpr uint16_t TELEMETRY_MS = 500;
constexpr const char *AUTH_NAMESPACE = "reflow_web";
constexpr const char *AUTH_SALT_KEY = "salt";
constexpr const char *AUTH_HASH_KEY = "hash";
constexpr const char *SETUP_AP_SSID_KEY = "ap_ssid";
constexpr const char *SETUP_AP_PASSWORD_KEY = "ap_pass";
constexpr const char *TOKEN_HEADER = "X-Reflow-Token";
constexpr const char *MDNS_NAME = "reflowdesk";
constexpr size_t MAX_PIN_LEN = 8;
constexpr size_t MIN_PIN_LEN = 6;
constexpr uint32_t WIFI_SETUP_PORTAL_TIMEOUT_MS = 180000UL;
constexpr size_t MIN_AP_PASSWORD_LEN = 8;
constexpr size_t MAX_AP_PASSWORD_LEN = 63;
constexpr size_t MIN_AP_SSID_LEN = 1;
constexpr size_t MAX_AP_SSID_LEN = 32;
constexpr uint8_t MAX_WS_CLIENTS = WEBSOCKETS_SERVER_CLIENT_MAX;

const char *HEADER_KEYS[] = {TOKEN_HEADER};

template <typename T>
T clampLocal(T value, T low, T high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

bool bodyToJson(const String &body, JsonDocument &doc) {
  return !body.isEmpty() && deserializeJson(doc, body) == DeserializationError::Ok;
}

bool pinFormatIsValid(const String &pin) {
  if (pin.length() < MIN_PIN_LEN || pin.length() > MAX_PIN_LEN) {
    return false;
  }
  for (size_t i = 0; i < pin.length(); ++i) {
    char c = pin.charAt(i);
    if (c < '0' || c > '9') {
      return false;
    }
  }
  return true;
}

#if REFLOW_DEBUG
void printSettingsSaveLine(const __FlashStringHelper *source, const char *message, const SettingsData &data) {
  const ReflowProfile &profile = SettingsStore::activeProfile(data);
  Serial.print(F("EV settings source="));
  Serial.print(source);
  Serial.print(F(" msg=\""));
  Serial.print(message);
  Serial.print(F("\" profile=P"));
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

ReflowWebServer::ReflowWebServer()
    : _server(HTTP_PORT), _ws(WS_PORT) {}

void ReflowWebServer::begin(SettingsStore &settings, SensorManager &sensors, ReflowController &reflow,
                            HeaterController &heater, FanController &fan, AlertManager &alerts, UiManager &ui,
                            FanController *boardFan) {
  _settings = &settings;
  _sensors = &sensors;
  _reflow = &reflow;
  _heater = &heater;
  _fan = &fan;
  _alerts = &alerts;
  _ui = &ui;
  _boardFan = boardFan;
  _seenResetCount = settings.resetCount();
  _lastSettingsRevision = settings.revision();
  _lastState = reflow.state();
  _lastFault = reflow.faultReason();

  beginAuth();
  if (!LittleFS.begin(false)) {
#if REFLOW_DEBUG
    Serial.println(F("Web LittleFS mount failed"));
#endif
  }

  configureWiFi();
  configureRoutes();
  _server.begin();
  _ws.begin();
  _ws.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    handleWebSocketEvent(num, type, payload, length);
  });
  _ready = true;

#if REFLOW_DEBUG
  Serial.print(F("Web ready http://"));
  Serial.print(_ipAddress);
  Serial.print(F(" ws=:"));
  Serial.println(WS_PORT);
#endif
}

void ReflowWebServer::loop(uint32_t now) {
  if (!_ready) {
    return;
  }

  _server.handleClient();
  _ws.loop();

  if (_settings != nullptr && _settings->resetCount() != _seenResetCount) {
    _seenResetCount = _settings->resetCount();
    _lastSettingsRevision = _settings->revision();
    resetPin();
    broadcastEvent("warning", "Factory reset completed. Web PIN and WiFi credentials cleared. Rebooting.");
    _resetWiFiBeforeRestart = true;
    _restartPending = true;
    _restartAtMs = now + 900;
  }

  if (_settings != nullptr && _settings->revision() != _lastSettingsRevision) {
    _lastSettingsRevision = _settings->revision();
    _alerts->setBuzzerLevel(_settings->data().buzzerLevel);
    _alerts->setLedBrightness(_settings->data().ledBrightness);
    broadcastEvent("success", "Settings changed on device.");
    broadcastTelemetry(now, true);
  }

  ReflowState state = _reflow->state();
  FaultReason fault = _reflow->faultReason();
  if (!_stateSeen) {
    _stateSeen = true;
    _lastState = state;
    _lastFault = fault;
    _targetReachedState = state;
    _targetReachedSeen = false;
  } else if (state != _lastState || fault != _lastFault) {
    _targetReachedState = state;
    _targetReachedSeen = false;
    if (state == ReflowState::Preheat) {
      broadcastEvent("success", "Reflow process started.");
    } else if (state == ReflowState::Soak) {
      broadcastEvent("info", "Soak stage started.");
    } else if (state == ReflowState::Reflow) {
      broadcastEvent("info", "Reflow stage started.");
    } else if (state == ReflowState::Cooling) {
      broadcastEvent("success", "Reflow complete. Cooling started.");
    } else if (state == ReflowState::Aborted) {
      broadcastEvent("warning", "Reflow process aborted.");
    } else if (state == ReflowState::Fault) {
      broadcastEvent("error", _reflow->faultName());
    } else if (state == ReflowState::Idle && _lastState == ReflowState::Cooling) {
      broadcastEvent("success", "Plate reached safe touch temperature.");
    } else if (state == ReflowState::Idle && _lastState == ReflowState::Aborted) {
      broadcastEvent("success", "Abort cooldown complete. Ready for next reflow.");
    }
    _lastState = state;
    _lastFault = fault;
    broadcastTelemetry(now, true);
  }

  if (_reflow->isHeatingState() && _reflow->holding() &&
      (!_targetReachedSeen || _targetReachedState != state)) {
    if (state == ReflowState::Preheat) {
      broadcastEvent("success", "Preheat target reached. Holding stage.");
    } else if (state == ReflowState::Soak) {
      broadcastEvent("success", "Soak target reached. Holding stage.");
    } else if (state == ReflowState::Reflow) {
      broadcastEvent("success", "Reflow target reached. Holding stage.");
    }
    _targetReachedState = state;
    _targetReachedSeen = true;
  }

  if (now - _lastTelemetryMs >= TELEMETRY_MS) {
    broadcastTelemetry(now);
  }

  if (_restartPending && static_cast<int32_t>(now - _restartAtMs) >= 0) {
    if (_resetWiFiBeforeRestart) {
      resetWiFiCredentials();
      delay(100);
    }
    ESP.restart();
  }
}

void ReflowWebServer::configureWiFi() {
  WiFi.mode(WIFI_STA);
  String apName = setupApName();
  String apPassword = setupApPassword();

  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(20);
  wifiManager.setConfigPortalTimeout(WIFI_SETUP_PORTAL_TIMEOUT_MS / 1000UL);
  wifiManager.setDebugOutput(REFLOW_DEBUG != 0);
  _wifiSetupLedActive = !wifiManager.getWiFiIsSaved();

  if (_ui != nullptr) {
    _ui->showStatus("WiFi Setup", apName.c_str(), "Open 192.168.4.1");
  }

  bool connected = false;
  if (_wifiSetupLedActive) {
    wifiManager.setConfigPortalBlocking(false);
    connected = wifiManager.autoConnect(apName.c_str(), apPassword.c_str());
    uint32_t setupStartMs = millis();
    while (!connected && WiFi.status() != WL_CONNECTED &&
           millis() - setupStartMs < WIFI_SETUP_PORTAL_TIMEOUT_MS) {
      wifiManager.process();
      if (_alerts != nullptr) {
        _alerts->blinkStatusWhite(millis());
      }
      delay(10);
      connected = WiFi.status() == WL_CONNECTED;
    }
    wifiManager.stopConfigPortal();
  } else {
    connected = wifiManager.autoConnect(apName.c_str(), apPassword.c_str());
  }

  if (connected && WiFi.status() == WL_CONNECTED) {
    _wifiSetupLedActive = false;
    if (_alerts != nullptr) {
      _alerts->clearStatusLed();
    }
    _ipAddress = WiFi.localIP().toString();
    if (MDNS.begin(MDNS_NAME)) {
      MDNS.addService("http", "tcp", HTTP_PORT);
      MDNS.addService("ws", "tcp", WS_PORT);
    }
    if (_ui != nullptr) {
      _ui->showStatus("Web Interface", _ipAddress.c_str(), "reflowdesk.local");
    }
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str(), apPassword.c_str());
  _ipAddress = WiFi.softAPIP().toString();
  if (_ui != nullptr) {
    _ui->showStatus("Setup AP Active", apName.c_str(), _ipAddress.c_str());
  }
}

void ReflowWebServer::configureRoutes() {
  _server.collectHeaders(HEADER_KEYS, 1);

  _server.on("/api/auth/status", HTTP_GET, [this]() { handleAuthStatus(); });
  _server.on("/api/auth/setup", HTTP_POST, [this]() { handleAuthSetup(); });
  _server.on("/api/auth/login", HTTP_POST, [this]() { handleAuthLogin(); });
  _server.on("/api/auth/logout", HTTP_POST, [this]() { handleAuthLogout(); });
  _server.on("/api/device/info", HTTP_GET, [this]() { handleDeviceInfo(); });
  _server.on("/api/settings", HTTP_GET, [this]() { handleSettingsGet(); });
  _server.on("/api/settings", HTTP_PUT, [this]() { handleSettingsPut(); });
  _server.on("/api/profiles", HTTP_GET, [this]() { handleProfilesGet(); });
  _server.on("/api/reflow/start", HTTP_POST, [this]() { handleReflowStart(); });
  _server.on("/api/reflow/abort", HTTP_POST, [this]() { handleReflowAbort(); });
  _server.on("/api/reflow/ack", HTTP_POST, [this]() { handleReflowAck(); });
  _server.on("/api/device/reboot", HTTP_POST, [this]() { handleDeviceReboot(); });
  _server.on("/api/device/factory-reset", HTTP_POST, [this]() { handleFactoryReset(); });
  _server.on("/api/ota/firmware", HTTP_POST, [this]() { handleOtaFinish(); }, [this]() { handleOtaUpload(); });
  _server.onNotFound([this]() { handleNotFound(); });
}

void ReflowWebServer::handleNotFound() {
  String uri = _server.uri();
  if (_server.method() == HTTP_PUT && uri.startsWith("/api/profiles/")) {
    int slot = uri.substring(strlen("/api/profiles/")).toInt();
    if (slot >= 1 && slot <= REFLOW_PROFILE_COUNT) {
      handleProfilePut(static_cast<uint8_t>(slot - 1));
      return;
    }
  }
  if (uri.startsWith("/api/")) {
    sendError(404, "Endpoint not found");
    return;
  }
  handleStaticFile(uri);
}

void ReflowWebServer::handleStaticFile(const String &path) {
  String filePath = path;
  if (filePath == "/") {
    filePath = "/index.html";
  }
  if (filePath.endsWith("/")) {
    filePath += "index.html";
  }
  if (filePath.indexOf("..") >= 0) {
    _server.send(403, "text/plain", "Forbidden");
    return;
  }

  File file = LittleFS.open(filePath, "r");
  if (!file) {
    _server.send(404, "text/plain", "Not found");
    return;
  }
  _server.streamFile(file, contentTypeFor(filePath));
  file.close();
}

void ReflowWebServer::handleAuthStatus() {
  JsonDocument doc;
  doc["pinSet"] = pinIsSet();
  doc["authenticated"] = authenticatedRequest();
  doc["device"] = DeviceInfo::NAME;
  doc["model"] = DeviceInfo::MODEL;
  sendJson(200, doc);
}

void ReflowWebServer::handleAuthSetup() {
  const bool existingPin = pinIsSet();
  if (existingPin && !authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  JsonDocument doc;
  if (!bodyToJson(_server.arg("plain"), doc)) {
    sendError(400, "Invalid JSON");
    return;
  }
  if (existingPin) {
    String currentPin = doc["currentPin"] | "";
    if (!verifyPin(currentPin)) {
      sendError(401, "Current PIN is incorrect");
      return;
    }
  }
  String pin = doc["pin"] | "";
  if (!createPin(pin)) {
    sendError(400, "PIN must be 6 to 8 digits");
    return;
  }
  createSessionToken();
  JsonDocument out;
  out["ok"] = true;
  out["token"] = _sessionToken;
  out["message"] = existingPin ? "PIN updated" : "PIN saved";
  sendJson(200, out);
  broadcastEvent("success", existingPin ? "Web PIN updated." : "Web PIN configured.");
}

void ReflowWebServer::handleAuthLogin() {
  JsonDocument doc;
  if (!bodyToJson(_server.arg("plain"), doc)) {
    sendError(400, "Invalid JSON");
    return;
  }
  String pin = doc["pin"] | "";
  if (!verifyPin(pin)) {
    sendError(401, "Invalid PIN");
    return;
  }
  createSessionToken();
  JsonDocument out;
  out["ok"] = true;
  out["token"] = _sessionToken;
  out["message"] = "Logged in";
  sendJson(200, out);
}

void ReflowWebServer::handleAuthLogout() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  clearSessionToken();
  sendOk("Logged out");
}

void ReflowWebServer::handleDeviceInfo() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  JsonDocument doc;
  doc["name"] = DeviceInfo::NAME;
  doc["model"] = DeviceInfo::MODEL;
  doc["manufacturer"] = DeviceInfo::MANUFACTURER;
  doc["hardwareVersion"] = DeviceInfo::HW_VERSION;
  doc["firmwareVersion"] = DeviceInfo::FW;
  doc["ip"] = _ipAddress;
  doc["mac"] = WiFi.macAddress();
  doc["rssi"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
  doc["wifiMode"] = WiFi.getMode() == WIFI_AP ? "AP" : (WiFi.status() == WL_CONNECTED ? "STA" : "Offline");
  doc["webSocketPort"] = WS_PORT;
  doc["uptimeMs"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["flashSize"] = ESP.getFlashChipSize();
  doc["sketchSize"] = ESP.getSketchSize();
  doc["freeSketchSpace"] = ESP.getFreeSketchSpace();
  doc["littleFsTotal"] = LittleFS.totalBytes();
  doc["littleFsUsed"] = LittleFS.usedBytes();
  sendJson(200, doc);
}

void ReflowWebServer::handleSettingsGet() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  JsonDocument doc;
  addSettingsJson(doc.to<JsonObject>(), _settings->data());
  sendJson(200, doc);
}

void ReflowWebServer::handleSettingsPut() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  if (!safeForPowerAction() && _reflow->isActive()) {
    sendError(409, "Settings cannot be changed during an active reflow process");
    return;
  }
  JsonDocument doc;
  if (!bodyToJson(_server.arg("plain"), doc)) {
    sendError(400, "Invalid JSON");
    return;
  }

  SettingsData candidate = _settings->data();
  if (doc["selectedProfileIndex"].is<int>()) {
    candidate.selectedProfileIndex = clampLocal<int>(doc["selectedProfileIndex"].as<int>(), 0, REFLOW_PROFILE_COUNT - 1);
  }
  if (doc["safeTouchC"].is<int>()) {
    candidate.safeTouchC = clampI16(doc["safeTouchC"].as<int>(), Limits::SAFE_TOUCH_MIN_C, Limits::SAFE_TOUCH_MAX_C);
  }
  if (doc["safetyCutoffC"].is<int>()) {
    candidate.safetyCutoffC = clampI16(doc["safetyCutoffC"].as<int>(), Limits::SAFETY_MIN_C, Limits::SAFETY_MAX_C);
  }
  if (doc["buzzerLevel"].is<int>()) {
    candidate.buzzerLevel = clampLocal<int>(doc["buzzerLevel"].as<int>(), 0, 5);
  } else if (doc["buzzerEnabled"].is<bool>()) {
    candidate.buzzerLevel = doc["buzzerEnabled"].as<bool>() ? 3 : 0;
  }
  if (doc["ledBrightness"].is<int>()) {
    int ledBrightness = clampLocal<int>(doc["ledBrightness"].as<int>(), 0, 100);
    candidate.ledBrightness = clampLocal<int>(((ledBrightness + 2) / 5) * 5, 0, 100);
  }
  if (doc["oledSleepTimeoutSeconds"].is<int>()) {
    candidate.oledSleepTimeoutSeconds =
        SettingsStore::normalizeOledSleepTimeoutSeconds(doc["oledSleepTimeoutSeconds"].as<int>());
  }
  if (doc["oledBrightness"].is<int>()) {
    candidate.oledBrightness = SettingsStore::normalizeOledBrightness(doc["oledBrightness"].as<int>());
  }
  if (doc["deviceControlsLocked"].is<bool>()) {
    candidate.deviceControlsLocked = doc["deviceControlsLocked"].as<bool>() ? 1 : 0;
  }
  if (doc["oledForcedOff"].is<bool>()) {
    candidate.oledForcedOff = doc["oledForcedOff"].as<bool>() ? 1 : 0;
  }
  if (doc["kp"].is<float>()) {
    candidate.kpX100 = clampI16(static_cast<int>(doc["kp"].as<float>() * 100.0f + 0.5f), 0, 3000);
  }
  if (doc["ki"].is<float>()) {
    candidate.kiX100 = clampI16(static_cast<int>(doc["ki"].as<float>() * 100.0f + 0.5f), 0, 500);
  }
  if (doc["kd"].is<float>()) {
    candidate.kdX100 = clampI16(static_cast<int>(doc["kd"].as<float>() * 100.0f + 0.5f), 0, 20000);
  }
  bool updateApName = false;
  String apName;
  if (doc["setupApSsid"].is<const char *>()) {
    apName = doc["setupApSsid"].as<const char *>();
    apName.trim();
    if (!setupApNameIsValid(apName)) {
      sendError(400, "Setup AP SSID must be 1 to 32 characters");
      return;
    }
    updateApName = true;
  }
  bool updateApPassword = false;
  String apPassword;
  if (doc["setupApPassword"].is<const char *>()) {
    apPassword = doc["setupApPassword"].as<const char *>();
    if (!setupApPasswordIsValid(apPassword)) {
      sendError(400, "Setup AP password must be 8 to 63 characters");
      return;
    }
    updateApPassword = true;
  }

  if (updateApName && !saveSetupApName(apName)) {
    sendError(500, "Failed to save setup AP SSID");
    return;
  }
  if (updateApPassword && !saveSetupApPassword(apPassword)) {
    sendError(500, "Failed to save setup AP password");
    return;
  }
#if REFLOW_DEBUG
  if (updateApName) {
    Serial.print(F("EV web setupAp ssid=\""));
    Serial.print(apName);
    Serial.println(F("\""));
  }
  if (updateApPassword) {
    Serial.println(F("EV web setupAp password=updated"));
  }
#endif

  applySettings(candidate, "Settings saved from web interface.");
}

void ReflowWebServer::handleProfilesGet() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  JsonDocument doc;
  doc["selectedProfileIndex"] = _settings->data().selectedProfileIndex;
  JsonArray profiles = doc["profiles"].to<JsonArray>();
  addProfilesJson(profiles, _settings->data());
  sendJson(200, doc);
}

void ReflowWebServer::handleProfilePut(uint8_t slot) {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  if (_reflow->isActive()) {
    sendError(409, "Profiles cannot be changed during an active reflow process");
    return;
  }
  JsonDocument doc;
  if (!bodyToJson(_server.arg("plain"), doc)) {
    sendError(400, "Invalid JSON");
    return;
  }

  SettingsData candidate = _settings->data();
  ReflowProfile &profile = candidate.profiles[slot];
  if (doc["name"].is<const char *>()) {
    const char *name = doc["name"].as<const char *>();
    if (name != nullptr && name[0] != '\0') {
      strncpy(profile.name, name, sizeof(profile.name) - 1);
      profile.name[sizeof(profile.name) - 1] = '\0';
    }
  }
  if (doc["preheatTempC"].is<int>()) {
    profile.preheatTempC = clampI16(doc["preheatTempC"].as<int>(), Limits::PREHEAT_MIN_C, Limits::PREHEAT_MAX_C);
  }
  if (doc["preheatSeconds"].is<int>()) {
    profile.preheatSeconds = clampU16(doc["preheatSeconds"].as<int>(), Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  }
  if (doc["soakTempC"].is<int>()) {
    profile.soakTempC = clampI16(doc["soakTempC"].as<int>(), Limits::PREHEAT_MIN_C, Limits::SOAK_MAX_C);
  }
  if (doc["soakSeconds"].is<int>()) {
    profile.soakSeconds = clampU16(doc["soakSeconds"].as<int>(), Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  }
  if (doc["reflowTempC"].is<int>()) {
    profile.reflowTempC = clampI16(doc["reflowTempC"].as<int>(), Limits::PREHEAT_MIN_C, Limits::REFLOW_MAX_C);
  }
  if (doc["reflowSeconds"].is<int>()) {
    profile.reflowSeconds = clampU16(doc["reflowSeconds"].as<int>(), Limits::STAGE_TIME_MIN_S, Limits::STAGE_TIME_MAX_S);
  }
  if (doc["coolingProfile"].is<const char *>()) {
    profile.coolingProfile = coolingProfileFromText(doc["coolingProfile"], profile.coolingProfile);
  } else if (doc["coolingProfile"].is<int>()) {
    profile.coolingProfile = clampLocal<int>(doc["coolingProfile"].as<int>(), COOLING_PROFILE_RAPID, COOLING_PROFILE_SILENT);
  }

  applySettings(candidate, "Reflow profile saved from web interface.");
}

void ReflowWebServer::handleReflowStart() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  JsonDocument doc;
  SettingsData candidate = _settings->data();
  if (bodyToJson(_server.arg("plain"), doc) && doc["profileIndex"].is<int>()) {
    uint8_t index = clampLocal<int>(doc["profileIndex"].as<int>(), 0, REFLOW_PROFILE_COUNT - 1);
    if (candidate.selectedProfileIndex != index) {
      candidate.selectedProfileIndex = index;
      _settings->editable() = candidate;
      if (!_settings->save()) {
        sendError(500, "Failed to save selected profile");
        return;
      }
    }
  }
  const TemperatureSample &sample = _sensors->sample();
  if (!_reflow->canStart(_settings->data(), sample)) {
    sendError(409, "Reflow cannot start until sensors are healthy and plate is safe");
    return;
  }
#if REFLOW_DEBUG
  const SettingsData &activeSettings = _settings->data();
  const ReflowProfile &profile = SettingsStore::activeProfile(activeSettings);
  Serial.print(F("EV web cmd=start profile=P"));
  Serial.print(activeSettings.selectedProfileIndex + 1);
  Serial.print(F(" name=\""));
  Serial.print(profile.name);
  Serial.println(F("\""));
#endif
  if (!_reflow->start(_settings->data(), sample)) {
    sendError(409, "Reflow start failed");
    return;
  }
  sendOk("Reflow started");
  broadcastTelemetry(millis(), true);
}

void ReflowWebServer::handleReflowAbort() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  if (!_reflow->isHeatingState()) {
    sendError(409, "Emergency stop is only available during an active reflow stage");
    return;
  }
#if REFLOW_DEBUG
  Serial.print(F("EV web cmd=abort stage="));
  Serial.println(_reflow->stateName());
#endif
  _reflow->abort();
  sendOk("Reflow aborted");
  broadcastTelemetry(millis(), true);
}

void ReflowWebServer::handleReflowAck() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  uint32_t now = millis();
  const TemperatureSample &sample = _sensors->sample();
  if (!_reflow->canAcknowledge(_settings->data(), sample, now)) {
    sendError(409, "Fault cannot be acknowledged yet");
    return;
  }
#if REFLOW_DEBUG
  Serial.println(F("EV web cmd=ack"));
#endif
  _reflow->acknowledge(_settings->data(), sample);
  sendOk("Fault acknowledged");
  broadcastTelemetry(now, true);
}

void ReflowWebServer::handleDeviceReboot() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  if (!safeForPowerAction()) {
    sendError(409, "Reboot is locked until the plate is idle and safe to touch");
    return;
  }
#if REFLOW_DEBUG
  Serial.println(F("EV web cmd=reboot"));
#endif
  sendOk("Rebooting");
  broadcastEvent("warning", "Device reboot requested from web interface.");
  _restartPending = true;
  _restartAtMs = millis() + 700;
}

void ReflowWebServer::handleFactoryReset() {
  if (!authenticatedRequest()) {
    sendError(401, "Authentication required");
    return;
  }
  JsonDocument doc;
  if (!bodyToJson(_server.arg("plain"), doc)) {
    sendError(400, "Invalid JSON");
    return;
  }
  String currentPin = doc["currentPin"] | "";
  if (!verifyPin(currentPin)) {
    sendError(401, "Current PIN is incorrect");
    return;
  }
  if (!safeForPowerAction()) {
    sendError(409, "Factory reset is locked until the plate is idle and safe to touch");
    return;
  }
#if REFLOW_DEBUG
  Serial.println(F("EV web cmd=factory-reset"));
#endif
  _settings->resetDefaults();
  bool saved = _settings->save();
  _lastSettingsRevision = _settings->revision();
  _seenResetCount = _settings->resetCount();
  resetPin();
  _alerts->setBuzzerLevel(_settings->data().buzzerLevel);
  _alerts->setLedBrightness(_settings->data().ledBrightness);
  if (!saved) {
    sendError(500, "Factory reset failed to save");
    return;
  }
  sendOk("Factory reset complete. Web PIN and WiFi credentials cleared. Rebooting.");
  broadcastEvent("warning", "Factory reset complete. Web PIN and WiFi credentials cleared.");
  broadcastTelemetry(millis(), true);
  _resetWiFiBeforeRestart = true;
  _restartPending = true;
  _restartAtMs = millis() + 900;
}

void ReflowWebServer::handleOtaFinish() {
  if (!_otaAuthorized) {
    sendError(401, "Authentication required");
    return;
  }
  if (_otaOk) {
    sendOk("Firmware uploaded. Rebooting.");
    broadcastEvent("success", "OTA firmware upload complete. Rebooting.");
#if REFLOW_DEBUG
    Serial.println(F("EV web ota=ok reboot=pending"));
#endif
    _restartPending = true;
    _restartAtMs = millis() + 1000;
  } else {
#if REFLOW_DEBUG
    Serial.println(F("EV web ota=failed"));
#endif
    sendError(500, "OTA upload failed");
  }
  _otaActive = false;
  _otaAuthorized = false;
}

void ReflowWebServer::handleOtaUpload() {
  HTTPUpload &upload = _server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    _otaAuthorized = authenticatedRequest() && safeForOta();
    _otaActive = _otaAuthorized;
    _otaOk = false;
    if (!_otaAuthorized) {
      return;
    }
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
      _otaActive = false;
      _otaAuthorized = false;
      return;
    }
#if REFLOW_DEBUG
    Serial.println(F("EV web ota=start"));
#endif
    broadcastEvent("info", "OTA firmware upload started.");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (_otaActive && Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      _otaActive = false;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (_otaActive) {
      _otaOk = Update.end(true);
      _otaActive = false;
    } else {
      Update.abort();
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    _otaActive = false;
    _otaOk = false;
#if REFLOW_DEBUG
    Serial.println(F("EV web ota=aborted"));
#endif
  }
}

void ReflowWebServer::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (num >= MAX_WS_CLIENTS) {
    return;
  }
  if (type == WStype_CONNECTED) {
    _wsAuthed[num] = false;
    JsonDocument doc;
    doc["type"] = "auth_required";
    doc["pinSet"] = pinIsSet();
    sendWsJson(num, doc);
    return;
  }
  if (type == WStype_DISCONNECTED) {
    _wsAuthed[num] = false;
    return;
  }
  if (type != WStype_TEXT || payload == nullptr || length == 0) {
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    return;
  }
  const char *msgType = doc["type"] | "";
  if (strcmp(msgType, "auth") == 0) {
    String token = doc["token"] | "";
    _wsAuthed[num] = validSessionToken(token);
    JsonDocument out;
    out["type"] = _wsAuthed[num] ? "auth_ok" : "auth_fail";
    sendWsJson(num, out);
    if (_wsAuthed[num]) {
      JsonDocument telemetry;
      telemetry["type"] = "telemetry";
      addTelemetryJson(telemetry["data"].to<JsonObject>(), millis());
      sendWsJson(num, telemetry);
    }
    return;
  }
  if (!_wsAuthed[num]) {
    JsonDocument out;
    out["type"] = "auth_required";
    sendWsJson(num, out);
    return;
  }
  if (strcmp(msgType, "ping") == 0) {
    JsonDocument out;
    out["type"] = "pong";
    out["uptimeMs"] = millis();
    sendWsJson(num, out);
  }
}

bool ReflowWebServer::beginAuth() {
  if (!_authPrefs.begin(AUTH_NAMESPACE, false)) {
    _pinSet = false;
    return false;
  }
  _pinSalt = _authPrefs.isKey(AUTH_SALT_KEY) ? _authPrefs.getString(AUTH_SALT_KEY, "") : "";
  _pinHash = _authPrefs.isKey(AUTH_HASH_KEY) ? _authPrefs.getString(AUTH_HASH_KEY, "") : "";
  _pinSet = _pinSalt.length() > 0 && _pinHash.length() > 0;
  return true;
}

String ReflowWebServer::setupApName() const {
  if (_setupApName.length() > 0) {
    return _setupApName;
  }
  String savedName =
      _authPrefs.isKey(SETUP_AP_SSID_KEY) ? _authPrefs.getString(SETUP_AP_SSID_KEY, "") : "";
  savedName.trim();
  if (setupApNameIsValid(savedName)) {
    _setupApName = savedName;
    return _setupApName;
  }
  uint8_t mac[6] = {};
  if (esp_read_mac(mac, ESP_MAC_WIFI_STA) != ESP_OK) {
    WiFi.macAddress(mac);
  }
  char name[33];
  snprintf(name, sizeof(name), "%s-%02X-%02X", REFLOW_WEB_SETUP_AP_BASE_SSID, mac[4], mac[5]);
  _setupApName = name;
  return _setupApName;
}

bool ReflowWebServer::setupApNameIsValid(const String &name) const {
  return name.length() >= MIN_AP_SSID_LEN && name.length() <= MAX_AP_SSID_LEN;
}

String ReflowWebServer::setupApPassword() const {
  String password =
      _authPrefs.isKey(SETUP_AP_PASSWORD_KEY) ? _authPrefs.getString(SETUP_AP_PASSWORD_KEY, "") : "";
  if (password.length() >= MIN_AP_PASSWORD_LEN && password.length() <= MAX_AP_PASSWORD_LEN) {
    return password;
  }
  return String(REFLOW_WEB_SETUP_AP_PASSWORD);
}

bool ReflowWebServer::setupApPasswordIsValid(const String &password) const {
  return password.length() >= MIN_AP_PASSWORD_LEN && password.length() <= MAX_AP_PASSWORD_LEN;
}

bool ReflowWebServer::saveSetupApName(const String &name) {
  String cleanName = name;
  cleanName.trim();
  if (!setupApNameIsValid(cleanName)) {
    return false;
  }
  _setupApName = cleanName;
  return _authPrefs.putString(SETUP_AP_SSID_KEY, cleanName) > 0;
}

bool ReflowWebServer::saveSetupApPassword(const String &password) {
  if (!setupApPasswordIsValid(password)) {
    return false;
  }
  return _authPrefs.putString(SETUP_AP_PASSWORD_KEY, password) > 0;
}

bool ReflowWebServer::validSessionToken(const String &token) const {
  return _sessionToken.length() > 0 && token == _sessionToken;
}

bool ReflowWebServer::authenticatedRequest() const {
  if (!pinIsSet()) {
    return false;
  }
  return validSessionToken(requestToken());
}

bool ReflowWebServer::authenticatedWs(uint8_t num) const {
  return num < MAX_WS_CLIENTS && _wsAuthed[num];
}

bool ReflowWebServer::createPin(const String &pin) {
  if (!pinFormatIsValid(pin)) {
    return false;
  }
  _pinSalt = randomHex(8);
  _pinHash = sha256Hex(_pinSalt + ":" + pin);
  _authPrefs.putString(AUTH_SALT_KEY, _pinSalt);
  _authPrefs.putString(AUTH_HASH_KEY, _pinHash);
  _pinSet = true;
  return true;
}

bool ReflowWebServer::verifyPin(const String &pin) const {
  if (!pinIsSet() || pin.length() == 0) {
    return false;
  }
  return sha256Hex(_pinSalt + ":" + pin) == _pinHash;
}

void ReflowWebServer::resetPin() {
  _authPrefs.remove(AUTH_SALT_KEY);
  _authPrefs.remove(AUTH_HASH_KEY);
  _pinSalt = "";
  _pinHash = "";
  _pinSet = false;
  clearSessionToken();
  memset(_wsAuthed, 0, sizeof(_wsAuthed));
}

void ReflowWebServer::createSessionToken() {
  _sessionToken = randomHex(16);
}

void ReflowWebServer::clearSessionToken() {
  _sessionToken = "";
}

bool ReflowWebServer::safeForPowerAction() const {
  uint32_t now = millis();
  const TemperatureSample &sample = _sensors->sample();
  return _reflow->state() == ReflowState::Idle && !_reflow->cooldownLocked(_settings->data(), sample, now);
}

bool ReflowWebServer::safeForOta() const {
  return safeForPowerAction() && !_otaActive;
}

void ReflowWebServer::applySettings(const SettingsData &candidate, const char *eventMessage) {
  _settings->editable() = candidate;
  if (!_settings->save()) {
    sendError(500, "Failed to save settings");
    return;
  }
  _alerts->setBuzzerLevel(_settings->data().buzzerLevel);
  _alerts->setLedBrightness(_settings->data().ledBrightness);
  _lastSettingsRevision = _settings->revision();
#if REFLOW_DEBUG
  printSettingsSaveLine(F("Web"), eventMessage, _settings->data());
#endif
  JsonDocument doc;
  doc["ok"] = true;
  doc["message"] = "Saved";
  addSettingsJson(doc["settings"].to<JsonObject>(), _settings->data());
  JsonArray profiles = doc["profiles"].to<JsonArray>();
  addProfilesJson(profiles, _settings->data());
  sendJson(200, doc);
  broadcastEvent("success", eventMessage);
  broadcastTelemetry(millis(), true);
}

void ReflowWebServer::resetWiFiCredentials() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  WiFi.disconnect(true, true);
  _authPrefs.remove(SETUP_AP_SSID_KEY);
  _authPrefs.remove(SETUP_AP_PASSWORD_KEY);
  _setupApName = "";
}

void ReflowWebServer::sendJson(int code, JsonDocument &doc) {
  String body;
  serializeJson(doc, body);
  _server.sendHeader("Cache-Control", "no-store");
  _server.send(code, "application/json", body);
}

void ReflowWebServer::sendError(int code, const char *message) {
  JsonDocument doc;
  doc["ok"] = false;
  doc["message"] = message;
  sendJson(code, doc);
}

void ReflowWebServer::sendOk(const char *message) {
  JsonDocument doc;
  doc["ok"] = true;
  doc["message"] = message;
  sendJson(200, doc);
}

void ReflowWebServer::broadcastTelemetry(uint32_t now, bool force) {
  if (!force && now - _lastTelemetryMs < TELEMETRY_MS) {
    return;
  }
  _lastTelemetryMs = now;

  bool hasAuthedClient = false;
  for (uint8_t i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (_wsAuthed[i]) {
      hasAuthedClient = true;
      break;
    }
  }
  if (!hasAuthedClient) {
    return;
  }

  JsonDocument doc;
  doc["type"] = "telemetry";
  addTelemetryJson(doc["data"].to<JsonObject>(), now);
  String body;
  serializeJson(doc, body);
  for (uint8_t i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (_wsAuthed[i]) {
      _ws.sendTXT(i, body);
    }
  }
}

void ReflowWebServer::broadcastEvent(const char *level, const char *message) {
  JsonDocument doc;
  doc["type"] = "event";
  doc["level"] = level;
  doc["message"] = message;
  doc["timeMs"] = millis();
  String body;
  serializeJson(doc, body);
  for (uint8_t i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (_wsAuthed[i]) {
      _ws.sendTXT(i, body);
    }
  }
}

void ReflowWebServer::sendWsJson(uint8_t num, JsonDocument &doc) {
  String body;
  serializeJson(doc, body);
  _ws.sendTXT(num, body);
}

void ReflowWebServer::addSettingsJson(JsonObject obj, const SettingsData &data) const {
  obj["selectedProfileIndex"] = data.selectedProfileIndex;
  obj["safeTouchC"] = data.safeTouchC;
  obj["safetyCutoffC"] = data.safetyCutoffC;
  obj["buzzerLevel"] = data.buzzerLevel;
  obj["buzzerEnabled"] = data.buzzerLevel != 0;
  obj["ledBrightness"] = data.ledBrightness;
  obj["oledSleepTimeoutSeconds"] = data.oledSleepTimeoutSeconds;
  obj["oledBrightness"] = data.oledBrightness;
  obj["oledBrightnessMin"] = Limits::OLED_BRIGHTNESS_MIN_PERCENT;
  obj["oledBrightnessMax"] = Limits::OLED_BRIGHTNESS_MAX_PERCENT;
  obj["oledBrightnessStep"] = Limits::OLED_BRIGHTNESS_STEP_PERCENT;
  obj["deviceControlsLocked"] = data.deviceControlsLocked != 0;
  obj["oledForcedOff"] = data.oledForcedOff != 0;
  obj["kp"] = SettingsStore::kp(data);
  obj["ki"] = SettingsStore::ki(data);
  obj["kd"] = SettingsStore::kd(data);
  obj["setupApSsid"] = setupApName();
  obj["setupApPassword"] = setupApPassword();
  obj["revision"] = _settings->revision();
}

void ReflowWebServer::addProfilesJson(JsonArray arr, const SettingsData &data) const {
  for (uint8_t i = 0; i < REFLOW_PROFILE_COUNT; ++i) {
    const ReflowProfile &profile = data.profiles[i];
    JsonObject p = arr.add<JsonObject>();
    p["slot"] = i + 1;
    p["index"] = i;
    p["name"] = profile.name;
    p["preheatTempC"] = profile.preheatTempC;
    p["preheatSeconds"] = profile.preheatSeconds;
    p["soakTempC"] = profile.soakTempC;
    p["soakSeconds"] = profile.soakSeconds;
    p["reflowTempC"] = profile.reflowTempC;
    p["reflowSeconds"] = profile.reflowSeconds;
    p["coolingProfile"] = normalizedCoolingName(profile.coolingProfile);
    p["jsonCrc"] = profile.jsonCrc;
  }
}

void ReflowWebServer::addTelemetryJson(JsonObject obj, uint32_t now) const {
  const SettingsData &data = _settings->data();
  const TemperatureSample &sample = _sensors->sample();
  const ReflowProfile &profile = SettingsStore::activeProfile(data);

  obj["timeMs"] = now;
  obj["state"] = _reflow->stateName();
  obj["fault"] = _reflow->faultName();
  obj["faultActive"] = _reflow->isFaultLike();
  obj["active"] = _reflow->isActive();
  obj["canAbort"] = _reflow->isHeatingState();
  obj["canStart"] = _reflow->canStart(data, sample);
  obj["canAcknowledge"] = _reflow->canAcknowledge(data, sample, now);
  obj["cooldownLocked"] = _reflow->cooldownLocked(data, sample, now);
  obj["targetC"] = _reflow->targetC(data);
  obj["holdElapsedSeconds"] = _reflow->holdElapsedSeconds(now);
  obj["holdTargetSeconds"] = _reflow->holdTargetSeconds(data);
  uint16_t holdTarget = _reflow->holdTargetSeconds(data);
  uint32_t holdElapsed = _reflow->holdElapsedSeconds(now);
  obj["timeLeftSeconds"] = holdTarget > holdElapsed ? holdTarget - holdElapsed : 0;
  obj["cooldownRemainingSeconds"] = _reflow->cooldownRemainingSeconds(now);

  obj["selectedProfileIndex"] = data.selectedProfileIndex;
  obj["profileName"] = profile.name;
  obj["settingsRevision"] = _settings->revision();

  JsonObject sensors = obj["sensors"].to<JsonObject>();
  sensors["plateC"] = sample.plateC;
  sensors["plateOk"] = sample.plateOk;
  sensors["ambientC"] = sample.ambientC;
  sensors["ambientOk"] = sample.ambientOk;
  sensors["boardC"] = sample.boardC;
  sensors["boardOk"] = sample.boardOk;
  sensors["boardPresent"] = REFLOW_HAS_BOARD_NTC != 0;
  sensors["plateRaw"] = sample.plateRaw;
  sensors["plateStatus"] = sample.plateStatus;

  JsonObject heater = obj["heater"].to<JsonObject>();
  heater["mode"] = _heater->modeName();
  heater["outputLabel"] = _heater->outputLabel();
  heater["dcPwm"] = _heater->dcPwmMode();
  heater["pwmHz"] = _heater->pwmFrequencyHz();
  heater["pwmBits"] = _heater->pwmResolutionBits();
  heater["enabled"] = _heater->enabled();
  heater["outputOn"] = _heater->outputOn();
  heater["ssrPinHigh"] = _heater->ssrCommandedOn();
  heater["commandedOn"] = _heater->heaterCommandedOn();
  heater["dutyPercent"] = _heater->dutyPercent();

  JsonObject fan = obj["fan"].to<JsonObject>();
  fan["on"] = _fan->isOn();
  fan["powerEnabled"] = _fan->powerEnabled();
  fan["speedPercent"] = _fan->speedPercent();
  fan["requestedPercent"] = _fan->requestedPercent();
  fan["rpm"] = _fan->rpm();
  fan["failed"] = _fan->failed();

  JsonObject boardFan = obj["boardFan"].to<JsonObject>();
  boardFan["present"] = _boardFan != nullptr;
  if (_boardFan != nullptr) {
    boardFan["on"] = _boardFan->isOn();
    boardFan["speedPercent"] = _boardFan->speedPercent();
    boardFan["rpm"] = _boardFan->rpm();
    boardFan["failed"] = _boardFan->failed();
  }
}

String ReflowWebServer::contentTypeFor(const String &path) const {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "text/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".svg")) return "image/svg+xml";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".woff2")) return "font/woff2";
  return "application/octet-stream";
}

String ReflowWebServer::requestToken() const {
  String header = _server.header(TOKEN_HEADER);
  if (header.length() > 0) {
    return header;
  }
  return _server.arg("token");
}

String ReflowWebServer::sha256Hex(const String &input) const {
  uint8_t hash[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, reinterpret_cast<const unsigned char *>(input.c_str()), input.length());
  mbedtls_sha256_finish(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  char out[65];
  for (uint8_t i = 0; i < sizeof(hash); ++i) {
    snprintf(out + i * 2, 3, "%02x", hash[i]);
  }
  out[64] = '\0';
  return String(out);
}

String ReflowWebServer::randomHex(uint8_t bytes) const {
  String out;
  out.reserve(bytes * 2);
  for (uint8_t i = 0; i < bytes; ++i) {
    uint8_t value = static_cast<uint8_t>(esp_random() & 0xFF);
    if (value < 16) {
      out += '0';
    }
    out += String(value, HEX);
  }
  return out;
}

String ReflowWebServer::normalizedCoolingName(uint8_t profile) const {
  switch (profile) {
    case COOLING_PROFILE_RAPID:
      return "rapid";
    case COOLING_PROFILE_SILENT:
      return "silent";
    case COOLING_PROFILE_NORMAL:
    default:
      return "normal";
  }
}

uint8_t ReflowWebServer::coolingProfileFromText(const char *name, uint8_t fallback) const {
  if (name == nullptr) {
    return fallback;
  }
  String text(name);
  text.toLowerCase();
  if (text == "rapid") return COOLING_PROFILE_RAPID;
  if (text == "normal") return COOLING_PROFILE_NORMAL;
  if (text == "silent") return COOLING_PROFILE_SILENT;
  return fallback;
}

int16_t ReflowWebServer::clampI16(int value, int16_t low, int16_t high) const {
  return static_cast<int16_t>(clampLocal<int>(value, low, high));
}

uint16_t ReflowWebServer::clampU16(int value, uint16_t low, uint16_t high) const {
  return static_cast<uint16_t>(clampLocal<int>(value, low, high));
}

#endif

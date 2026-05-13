/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * web_server.h: ESP32-S3 hosted web interface
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "alerts.h"
#include "config.h"
#include "fan.h"
#include "heater.h"
#include "reflow.h"
#include "sensors.h"
#include "settings.h"
#include "ui.h"

#if REFLOW_WEB_ENABLED
#include <Preferences.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

class ReflowWebServer {
public:
  ReflowWebServer();

  void begin(SettingsStore &settings, SensorManager &sensors, ReflowController &reflow, HeaterController &heater,
             FanController &fan, AlertManager &alerts, UiManager &ui, FanController *boardFan = nullptr);
  void loop(uint32_t now);
  bool ready() const { return _ready; }
  const char *ipAddress() const { return _ipAddress.c_str(); }

private:
  void configureWiFi();
  void configureRoutes();
  void handleNotFound();
  void handleStaticFile(const String &path);
  void handleAuthStatus();
  void handleAuthSetup();
  void handleAuthLogin();
  void handleAuthLogout();
  void handleDeviceInfo();
  void handleSettingsGet();
  void handleSettingsPut();
  void handleProfilesGet();
  void handleProfilePut(uint8_t slot);
  void handleReflowStart();
  void handleReflowAbort();
  void handleReflowAck();
  void handleDeviceReboot();
  void handleFactoryReset();
  void handleOtaFinish();
  void handleOtaUpload();
  void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

  bool beginAuth();
  String setupApName() const;
  String setupApPassword() const;
  bool setupApPasswordIsValid(const String &password) const;
  bool saveSetupApPassword(const String &password);
  bool pinIsSet() const { return _pinSet; }
  bool validSessionToken(const String &token) const;
  bool authenticatedRequest() const;
  bool authenticatedWs(uint8_t num) const;
  bool createPin(const String &pin);
  bool verifyPin(const String &pin) const;
  void resetPin();
  void createSessionToken();
  void clearSessionToken();

  bool safeForPowerAction() const;
  bool safeForOta() const;
  void applySettings(const SettingsData &settings, const char *eventMessage);
  void sendJson(int code, JsonDocument &doc);
  void sendError(int code, const char *message);
  void sendOk(const char *message);
  void broadcastTelemetry(uint32_t now, bool force = false);
  void broadcastEvent(const char *level, const char *message);
  void resetWiFiCredentials();
  void sendWsJson(uint8_t num, JsonDocument &doc);
  void addSettingsJson(JsonObject obj, const SettingsData &data) const;
  void addProfilesJson(JsonArray arr, const SettingsData &data) const;
  void addTelemetryJson(JsonObject obj, uint32_t now) const;
  String contentTypeFor(const String &path) const;
  String requestToken() const;
  String sha256Hex(const String &input) const;
  String randomHex(uint8_t bytes) const;
  String normalizedCoolingName(uint8_t profile) const;
  uint8_t coolingProfileFromText(const char *name, uint8_t fallback) const;
  int16_t clampI16(int value, int16_t low, int16_t high) const;
  uint16_t clampU16(int value, uint16_t low, uint16_t high) const;

  WebServer _server;
  WebSocketsServer _ws;
  mutable Preferences _authPrefs;
  SettingsStore *_settings = nullptr;
  SensorManager *_sensors = nullptr;
  ReflowController *_reflow = nullptr;
  HeaterController *_heater = nullptr;
  FanController *_fan = nullptr;
  FanController *_boardFan = nullptr;
  AlertManager *_alerts = nullptr;
  UiManager *_ui = nullptr;

  String _ipAddress;
  mutable String _setupApName;
  String _sessionToken;
  String _pinSalt;
  String _pinHash;
  bool _pinSet = false;
  bool _ready = false;
  bool _wsAuthed[WEBSOCKETS_SERVER_CLIENT_MAX] = {};
  uint32_t _lastTelemetryMs = 0;
  uint32_t _lastSettingsRevision = 0;
  uint32_t _seenResetCount = 0;
  ReflowState _lastState = ReflowState::Idle;
  FaultReason _lastFault = FaultReason::None;
  ReflowState _targetReachedState = ReflowState::Idle;
  bool _stateSeen = false;
  bool _targetReachedSeen = false;
  bool _otaAuthorized = false;
  bool _otaActive = false;
  bool _otaOk = false;
  bool _restartPending = false;
  bool _resetWiFiBeforeRestart = false;
  uint32_t _restartAtMs = 0;
};
#else
class ReflowWebServer {
public:
  void begin(SettingsStore &, SensorManager &, ReflowController &, HeaterController &, FanController &, AlertManager &,
             UiManager &, FanController * = nullptr) {}
  void loop(uint32_t) {}
  bool ready() const { return false; }
  const char *ipAddress() const { return ""; }
};
#endif

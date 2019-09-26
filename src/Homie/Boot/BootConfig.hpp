#pragma once

#include "Arduino.h"

#include "../Constants.hpp"
#if HOMIE_CONFIG

#include <functional>
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncTCP.h>
#endif // ESP32
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "Boot.hpp"
#include "../Limits.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Timer.hpp"
#include "../Utils/DeviceId.hpp"
#include "../Utils/Validation.hpp"
#include "../Utils/Helpers.hpp"
#include "../Logger.hpp"
#include "../Strings.hpp"
#include "../../HomieSetting.hpp"
#include "../../StreamingOperator.hpp"

namespace HomieInternals {
class BootConfig : public Boot {
 public:
  BootConfig();
  ~BootConfig();
  void setup();
  void loop();

 private:
  AsyncWebServer _http;
  HTTPClient _httpClient;
  DNSServer _dns;
  uint8_t _ssidCount;
  bool _wifiScanAvailable;
  Timer _wifiScanTimer;
  bool _lastWifiScanEnded;
  String _jsonWifiNetworks;
  bool _flaggedForReboot;
  uint32_t _flaggedForRebootAt;
  bool _proxyEnabled;
  char _apIpStr[MAX_IP_STRING_LENGTH];

  void _onCaptivePortal(AsyncWebServerRequest *request);
  void _onDeviceInfoRequest(AsyncWebServerRequest *request);
  void _onNetworksRequest(AsyncWebServerRequest *request);
  void _onConfigRequest(AsyncWebServerRequest *request);
  void _generateNetworksJson();
  void _onWifiConnectRequest(AsyncWebServerRequest *request);
  void _onProxyControlRequest(AsyncWebServerRequest *request);
  void _proxyHttpRequest(AsyncWebServerRequest *request);
  void _onWifiStatusRequest(AsyncWebServerRequest *request);

  // Helpers
  static void __setCORS();
  static const int MAX_POST_SIZE = 1500;
  static void __parsePost(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  static void __SendJSONError(AsyncWebServerRequest *request, String msg, int16_t code = 400);
};
}  // namespace HomieInternals

#endif

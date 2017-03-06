#pragma once

#include "Arduino.h"

#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "Boot.hpp"
#include "../Constants.hpp"
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
  HTTPClient _httpClient;
  ESP8266WebServer _http;
  DNSServer _dns;
  uint8_t _ssidCount;
  bool _wifiScanAvailable;
  Timer _wifiScanTimer;
  bool _lastWifiScanEnded;
  char* _jsonWifiNetworks;
  bool _flaggedForReboot;
  uint32_t _flaggedForRebootAt;
  bool _proxyEnabled;
  char _apIpStr[MAX_IP_STRING_LENGTH];

  void _onCaptivePortal();
  void _onDeviceInfoRequest();
  void _onNetworksRequest();
  void _onConfigRequest();
  void _generateNetworksJson();
  void _onWifiConnectRequest();
  void _onProxyControlRequest();
  void _proxyHttpRequest();
  void _onWifiStatusRequest();
};
}  // namespace HomieInternals

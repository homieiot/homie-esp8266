#pragma once

#include <Arduino.h>
#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "Boot.hpp"
#include "../Config.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Helpers.hpp"
#include "../Logger.hpp"
#include "../Strings.hpp"

namespace HomieInternals {
  class BootConfig : public Boot {
    public:
      BootConfig(Interface* interface);
      ~BootConfig();
      void setup();
      void loop();
    private:
      Interface* _interface;
      ESP8266WebServer _http;
      DNSServer _dns;
      byte _ssidCount;
      bool _wifiScanAvailable;
      unsigned long _lastWifiScan;
      bool _lastWifiScanEnded;
      char* _jsonWifiNetworks;
      bool _flaggedForReboot;
      unsigned long _flaggedForRebootAt;

      void _onDeviceInfoRequest();
      void _onNetworksRequest();
      void _onConfigRequest();
      void _generateNetworksJson();
  };
}

#pragma once

#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "Boot.hpp"
#include "../Config.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Timer.hpp"
#include "../Helpers.hpp"
#include "../Logger.hpp"
#include "../Strings.hpp"

namespace HomieInternals {
  class BootConfig : public Boot {
    public:
      BootConfig();
      ~BootConfig();
      void setup();
      void loop();
    private:
      ESP8266WebServer _http;
      DNSServer _dns;
      unsigned char _ssidCount;
      bool _wifiScanAvailable;
      Timer _wifiScanTimer;
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

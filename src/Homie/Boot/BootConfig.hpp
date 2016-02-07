#pragma once

#include <Arduino.h>
#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "Boot.hpp"
#include "../Config.hpp"
#include "../Datatypes/SharedInterface.hpp"
#include "../Helpers.hpp"
#include "../Logger.hpp"
#include "../Strings.hpp"

namespace HomieInternals {
  class BootConfig : public Boot {
    public:
      BootConfig(SharedInterface* shared_interface);
      ~BootConfig();
      void setup();
      void loop();
    private:
      SharedInterface* _shared_interface;
      ESP8266WebServer _http;
      DNSServer _dns;
      byte _ssid_count;
      unsigned long _last_wifi_scan;
      bool _last_wifi_scan_ended;
      String _json_wifi_networks;
      bool _flagged_for_reboot;
      unsigned long _flagged_for_reboot_at;

      void _onDeviceInfoRequest();
      void _onNetworksRequest();
      void _onConfigRequest();
      String _generateNetworksJson();
  };
}

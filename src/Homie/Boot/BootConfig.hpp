#pragma once

#include <Arduino.h>
#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "../../3rd/ArduinoJson/src/ArduinoJson.h"
#include <DNSServer.h>
#include "Boot.hpp"
#include "../Config.hpp"
#include "../Strings.hpp"

class BootConfig : public Boot {
  public:
    BootConfig();
    ~BootConfig();
    void setup();
    void loop();
  private:
    ESP8266WebServer _http;
    DNSServer _dns;
    byte _ssid_count;
    unsigned long _last_wifi_scan;
    bool _last_wifi_scan_ended;
    String _json_wifi_networks;

    void _onNetworksRequest();
    void _onConfigRequest();
    String _generateNetworksJson();
};

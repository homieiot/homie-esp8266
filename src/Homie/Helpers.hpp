#pragma once

#include "Arduino.h"

#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "Limits.hpp"

namespace HomieInternals {
  struct ConfigValidationResult {
    bool valid;
    const __FlashStringHelper* reason;
  };

  struct MdnsQueryResult {
    bool success;
    IPAddress ip;
    unsigned int port;
  };

  class Helpers {
    public:
      static void generateDeviceId();
      static const char* getDeviceId();
      static MdnsQueryResult mdnsQuery(const char* service);
      static ConfigValidationResult validateConfig(const JsonObject& object);

    private:
      static char _deviceId[8 + 1];

      static ConfigValidationResult _validateConfigRoot(const JsonObject& object);
      static ConfigValidationResult _validateConfigWifi(const JsonObject& object);
      static ConfigValidationResult _validateConfigMqtt(const JsonObject& object);
      static ConfigValidationResult _validateConfigOta(const JsonObject& object);
  };
}

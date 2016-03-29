#pragma once

#include <ArduinoJson.h>
#include "Limits.hpp"
#include "Logger.hpp"

namespace HomieInternals {
  class Helpers {
    public:
      static void generateDeviceId();
      static const char* getDeviceId();
      static bool validateConfig(const JsonObject& object);

    private:
      static char _deviceId[8 + 1];

      static bool _validateConfigRoot(const JsonObject& object);
      static bool _validateConfigWifi(const JsonObject& object);
      static bool _validateConfigMqtt(const JsonObject& object);
      static bool _validateConfigOta(const JsonObject& object);
  };
}

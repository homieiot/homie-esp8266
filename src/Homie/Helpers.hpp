#pragma once

#include <Arduino.h>

#include <ArduinoJson.h>
#include "Limits.hpp"
#include "Logger.hpp"

namespace HomieInternals {
  class Helpers {
    public:
      static void generateDeviceId();
      static const char* getDeviceId();
      static bool validateConfig(JsonObject& object);

    private:
      static char _deviceId[8 + 1];
  };
}

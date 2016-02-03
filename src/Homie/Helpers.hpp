#pragma once

#include <Arduino.h>

#include "../3rd/ArduinoJson/src/ArduinoJson.h"
#include "Logger.hpp"

namespace HomieInternals {
  class Helpers {
    public:
      static String getDeviceId();
      static bool validateConfig(JsonObject& object);
  };
}

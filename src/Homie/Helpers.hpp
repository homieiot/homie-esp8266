#pragma once

#include <Arduino.h>

#include <ArduinoJson.h>
#include "Logger.hpp"

namespace HomieInternals {
  class HelpersClass {
    public:
      HelpersClass();
      const char* getDeviceId();
      bool validateConfig(JsonObject& object);

    private:
      char _device_id[8 + 1];
  };

  extern HelpersClass Helpers;
}

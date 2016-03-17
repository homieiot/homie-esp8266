#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../Datatypes/SharedInterface.hpp"
#include "../Constants.hpp"
#include "../Logger.hpp"
#include "../Helpers.hpp"

namespace HomieInternals {
  class Boot {
    public:
      Boot(SharedInterface* sharedInterface, const char* name);
      virtual void setup();
      virtual void loop();
    private:
      SharedInterface* _sharedInterface;
      const char* _name;
  };
}

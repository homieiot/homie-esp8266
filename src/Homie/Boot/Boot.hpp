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
      Boot(SharedInterface* shared_interface, const char* name);
      virtual void setup();
      virtual void loop();
    private:
      SharedInterface* _shared_interface;
      const char* _name;
  };
}

#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../Datatypes/Interface.hpp"
#include "../Constants.hpp"
#include "../Logger.hpp"
#include "../Helpers.hpp"

namespace HomieInternals {
  class Boot {
    public:
      Boot(Interface* interface, const char* name);
      virtual void setup();
      virtual void loop();
    private:
      Interface* _interface;
      const char* _name;
  };
}

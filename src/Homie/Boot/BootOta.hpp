#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include "../Constants.hpp"
#include "../Datatypes/SharedInterface.hpp"
#include "../Config.hpp"
#include "../Logger.hpp"
#include "Boot.hpp"

namespace HomieInternals {
  class BootOta : public Boot {
    public:
      BootOta(SharedInterface* sharedInterface);
      ~BootOta();
      void setup();
      void loop();

    private:
      SharedInterface* _sharedInterface;
  };
}

#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include "../Constants.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Config.hpp"
#include "../Logger.hpp"
#include "Boot.hpp"

namespace HomieInternals {
  class BootOta : public Boot {
    public:
      BootOta();
      ~BootOta();
      void setup();
      void loop();

    private:
  };
}

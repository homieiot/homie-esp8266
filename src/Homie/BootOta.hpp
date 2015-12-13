#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include "Constants.hpp"
#include "Config.hpp"
#include "Boot.hpp"

class BootOta : public Boot {
  public:
    BootOta(SharedInterface* shared_interface);
    ~BootOta();
    void setup();
    void loop();

  private:
    SharedInterface* _shared_interface;
};

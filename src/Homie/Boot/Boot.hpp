#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../Constants.hpp"
#include "../Logger.hpp"

class Boot {
  public:
    Boot(const char* name);
    virtual void setup();
    virtual void loop();
  private:
    const char* _name;
};

#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "Datatypes/ConfigStruct.hpp"
#include "Helpers.hpp"
#include "Logger.hpp"

namespace HomieInternals {
  class ConfigClass {
    public:
      ConfigClass();
      bool load();
      ConfigStruct& get();
      void erase();
      void write(const String& config);
      void setOtaMode(bool enabled);
      BootMode getBootMode();
      void log(); // print the current config to log output

    private:
      BootMode _boot_mode;
      ConfigStruct _config_struct;
      bool _spiffs_began;

      bool _spiffsBegin();
  };

  extern ConfigClass Config;
}

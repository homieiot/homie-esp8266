#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "Datatypes/ConfigStruct.hpp"
#include "Logger.hpp"

namespace HomieInternals {
  class ConfigClass {
    public:
      ConfigClass();
      bool load();
      ConfigStruct& get();
      void save();
      void setCustomEepromSize(int count);
      void log(); // print the current config to log output

    private:
      ConfigStruct _config_struct;
      bool _eeprom_began;
      int _custom_eeprom_size;

      void _eepromBegin();
  };

  extern ConfigClass Config;
}

#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "Constants.hpp"
#include "Logger.hpp"

namespace HomieInternals {
  class ConfigClass {
    public:
      bool configured;
      BootMode boot_mode;
      const char* hostname;
      const char* wifi_ssid;
      const char* wifi_password;
      const char* homie_host;
      uint16_t homie_port;
      const char* homie_ota_path;
      uint16_t homie_ota_port;

      ConfigClass();
      bool load();
      void save();
      void setCustomEepromSize(int count);
      void log();       // print the current config to log output

    private:
      ConfigStruct _config_struct;
      bool _eeprom_began;
      int _custom_eeprom_size;

      void _eepromBegin();
  };

  extern ConfigClass Config;
}

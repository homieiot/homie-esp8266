#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "Constants.hpp"

class ConfigClass {
  public:
    bool configured;
    BootMode boot_mode;
    const char* hostname;
    const char* wifi_ssid;
    const char* wifi_password;
    const char* homie_host;

    ConfigClass();
    bool load();
    void save();
    void setCustomEepromSize(int count);

  private:
    ConfigStruct _config_struct;
    bool _eeprom_began;
    int _custom_eeprom_size;

    void _eepromBegin();
};

extern ConfigClass Config;

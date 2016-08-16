#pragma once

#include "Arduino.h"

#include <ArduinoJson.h>
#include "FS.h"
#include "Datatypes/Interface.hpp"
#include "Datatypes/ConfigStruct.hpp"
#include "Helpers.hpp"
#include "Limits.hpp"
#include "Logger.hpp"
#include "../HomieSetting.hpp"

namespace HomieInternals {
class Config {
 public:
  Config();
  void attachInterface(Interface* interface);
  bool load();
  inline const ConfigStruct& get() const;
  char* getSafeConfigFile() const;
  void erase();
  void write(const char* config);
  BootMode getBootMode() const;
  void log() const;  // print the current config to log output

 private:
  Interface* _interface;
  BootMode _bootMode;
  ConfigStruct _configStruct;
  bool _spiffsBegan;

  bool _spiffsBegin();
};

const ConfigStruct& Config::get() const {
  return _configStruct;
}
}  // namespace HomieInternals

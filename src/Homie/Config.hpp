#pragma once

#include <ArduinoJson.h>
#include "FS.h"
#include "Datatypes/Interface.hpp"
#include "Datatypes/ConfigStruct.hpp"
#include "Helpers.hpp"
#include "Limits.hpp"
#include "Logger.hpp"

namespace HomieInternals {
  class Config {
    public:
      Config();
      void attachInterface(Interface* interface);
      bool load();
      const ConfigStruct& get();
      void erase();
      void write(const String& config);
      void setOtaMode(bool enabled, const char* version = "");
      const char* getOtaVersion();
      BootMode getBootMode();
      void log(); // print the current config to log output

    private:
      Interface* _interface;
      BootMode _bootMode;
      ConfigStruct _configStruct;
      char _otaVersion[MAX_FIRMWARE_VERSION_LENGTH];
      bool _spiffsBegan;

      bool _spiffsBegin();
  };
}

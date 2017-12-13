#pragma once

#include "Arduino.h"

#include <ArduinoJson.h>
#include "FS.h"
#include "Datatypes/Interface.hpp"
#include "Datatypes/ConfigStruct.hpp"
#include "Utils/DeviceId.hpp"
#include "Utils/Validation.hpp"
#include "Constants.hpp"
#include "Limits.hpp"
#include "../HomieBootMode.hpp"
#include "../HomieSetting.hpp"
#include "../StreamingOperator.hpp"
#include "Strings.hpp"

namespace HomieInternals {
class Config {
 public:
  Config();
  bool load();
  const ConfigStruct& get() const;
  char* getSafeConfigFile() const;
  void erase();
  void setHomieBootModeOnNextBoot(HomieBootMode bootMode);
  HomieBootMode getHomieBootModeOnNextBoot();
  ConfigValidationResult write(const JsonObject& config);
  ConfigValidationResult patch(const char* patch);
  void log() const;  // print the current config to log output
  bool isValid() const;

  static ConfigValidationResult validateConfig(const JsonObject& parsedJson);

 private:
  ConfigStruct _configStruct;
  bool _spiffsBegan;
  bool _valid;

  bool _spiffsBegin();
  ConfigValidationResultOBJ _loadConfigFile(StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE>* buf);
};
}  // namespace HomieInternals

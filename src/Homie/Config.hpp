#pragma once

#include "Arduino.h"

#include <ArduinoJson.h>
#include "FS.h"
#include "./Datatypes/Interface.hpp"
#include "./Datatypes/ConfigStruct.hpp"
#include "./Utils/DeviceId.hpp"
#include "./Utils/Validation.hpp"
#include "./Constants.hpp"
#include "./Limits.hpp"
#include "../HomieBootMode.hpp"
#include "../HomieSetting.hpp"
#include "../StreamingOperator.hpp"
#include "./Strings.hpp"
#include "./Datatypes/Result.hpp"

namespace HomieInternals {
class Config {
 public:
  Config();
  bool load();
  const ConfigStruct& get() const;
  ValidationResultOBJ getConfigFile(StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE>* jsonBuffer);
  ValidationResultOBJ getSafeConfigFile(StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE>* jsonBuffer);
  std::unique_ptr<char[]> getSafeConfigFileSTR();
  void erase();
  void setHomieBootModeOnNextBoot(HomieBootMode bootMode);
  HomieBootMode getHomieBootModeOnNextBoot();
  ValidationResult write(const JsonObject& config);
  ValidationResult patch(const char* patch);
  template<class T>
  ValidationResult saveSetting(const char* name, T value);
  void log() const;  // print the current config to log output
  bool isValid() const;

  static ValidationResult validateConfig(const JsonObject& parsedJson, bool skipValidation = false);

 private:
  ConfigStruct _configStruct;
  bool _spiffsBegan;
  bool _valid;

  bool _spiffsBegin();
  ValidationResultOBJ _loadConfigFile(StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE>* jsonBuffer, bool skipValidation = false);
};

}  // namespace HomieInternals

// Template Implementations
using namespace HomieInternals;

template<class T>
ValidationResult Config::saveSetting(const char* name, T value) {
  ValidationResult result;
  result.valid = false;

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE> currentJsonBuffer;
  ValidationResultOBJ configLoadResult = _loadConfigFile(&currentJsonBuffer, true);
  if (!configLoadResult.valid) {
    result.reason = F("✖ Old Config file is not valid, reason: ");
    result.reason.concat(configLoadResult.reason);
    return result;
  }
  JsonObject& configObject = *configLoadResult.config;

  if (!configObject.containsKey("settings")) {
    configObject.createNestedObject("settings");
  }
  if (!configObject["settings"].is<JsonObject>()) {
    configObject.remove("settings");
    configObject.createNestedObject("settings");
  }

  JsonObject& settings = configObject["settings"].as<JsonObject>();
  settings[name] = value;

  ValidationResult configWriteResult = write(configObject);
  if (!configWriteResult.valid) {
    result.reason = F("✖ New Config file is not valid, reason: ");
    result.reason.concat(configWriteResult.reason);
    return result;
  }

  result.valid = true;
  return result;
}

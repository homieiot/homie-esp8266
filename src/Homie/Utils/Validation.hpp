#pragma once

#include "Arduino.h"

#include <ArduinoJson.h>
#include "Helpers.hpp"
#include "../Limits.hpp"
#include "../../HomieSetting.hpp"

namespace HomieInternals {
struct ConfigValidationResult {
  bool valid;
  String reason;
};

class Validation {
 public:
  static ConfigValidationResult validateConfig(const JsonObject object);

 private:
  static ConfigValidationResult _validateConfigRoot(const JsonObject object);
  static ConfigValidationResult _validateConfigWifi(const JsonObject object);
  static ConfigValidationResult _validateConfigMqtt(const JsonObject object);
  static ConfigValidationResult _validateConfigOta(const JsonObject object);
  static ConfigValidationResult _validateConfigSettings(const JsonObject object);
};
}  // namespace HomieInternals

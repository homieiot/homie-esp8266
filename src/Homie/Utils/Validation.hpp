#pragma once

#include "Arduino.h"

#include <ArduinoJson.h>
#include "Helpers.hpp"
#include "../Limits.hpp"
#include "../../HomieSetting.hpp"
#include "../Datatypes/Result.hpp"

namespace HomieInternals {
class Validation {
 public:
  static ValidationResult validateConfig(const JsonObject& object);

 private:
  static ValidationResult _validateConfigRoot(const JsonObject& object);
  static ValidationResult _validateConfigWifi(const JsonObject& object);
  static ValidationResult _validateConfigMqtt(const JsonObject& object);
  static ValidationResult _validateConfigOta(const JsonObject& object);
  static ValidationResult _validateConfigSettings(const JsonObject& object);
};
}  // namespace HomieInternals

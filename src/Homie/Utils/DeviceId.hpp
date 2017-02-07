#pragma once

#include "Arduino.h"

#include <ESP8266WiFi.h>

#include "../Limits.hpp"

namespace HomieInternals {
class DeviceId {
 public:
  static void generate();
  static const char* get();

 private:
  static char _deviceId[MAX_MAC_STRING_LENGTH + 1];
};
}  // namespace HomieInternals

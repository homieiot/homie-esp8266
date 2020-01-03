#pragma once

#include "Arduino.h"

#ifdef ESP32
#include <WiFi.h>
#include "esp_system.h"
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif // ESP32

#include "../Limits.hpp"

namespace HomieInternals {
class DeviceId {
 public:
  static void generate();
  static const char* get();

 private:
  static char _deviceId[MAX_MAC_STRING_LENGTH];
};
}  // namespace HomieInternals

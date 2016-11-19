#pragma once

#include "Arduino.h"

#include <ESP8266WiFi.h>

namespace HomieInternals {
class DeviceId {
 public:
  static void generate();
  static const char* get();

 private:
  static char _deviceId[12 + 1];
};
}  // namespace HomieInternals

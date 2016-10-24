#pragma once

#include "Arduino.h"

namespace HomieInternals {
class DeviceId {
 public:
  static void generate();
  static const char* get();

 private:
  static char _deviceId[8 + 1];
};
}  // namespace HomieInternals

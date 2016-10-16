#pragma once

#include "Arduino.h"

namespace HomieInternals {
class Helpers {
 public:
  static uint8_t rssiToPercentage(int32_t rssi);
};
}  // namespace HomieInternals

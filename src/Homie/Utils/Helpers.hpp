#pragma once

#include "Arduino.h"

#include <memory>

namespace HomieInternals {
class Helpers {
 public:
  static uint8_t rssiToPercentage(int32_t rssi);
  static std::unique_ptr<char[]> cloneString(const String& string);
};
}  // namespace HomieInternals

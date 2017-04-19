#pragma once

#include "Arduino.h"
#include "../Limits.hpp"
#include <memory>

namespace HomieInternals {
class Helpers {
 public:
  static uint8_t rssiToPercentage(int32_t rssi);
  static void stringToBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);
  static bool validateMacAddress(const char* mac);
  static std::unique_ptr<char[]> cloneString(const String& string);
};
}  // namespace HomieInternals

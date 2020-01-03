#pragma once

#include "Arduino.h"
#include <IPAddress.h>
#include "../../StreamingOperator.hpp"
#include "../Limits.hpp"
#include <memory>

namespace HomieInternals {
class Helpers {
 public:
  static void abort(const String& message);
  static uint8_t rssiToPercentage(int32_t rssi);
  static void stringToBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);
  static bool validateIP(const char* ip);
  static bool validateMacAddress(const char* mac);
  static bool validateMd5(const char* md5);
  static std::unique_ptr<char[]> cloneString(const String& string);
  static void ipToString(const IPAddress& ip, char* str);
  static void hexStringToByteArray(const char* hexStr, uint8_t* hexArray, uint8_t size);
  static void byteArrayToHexString(const uint8_t* hexArray, char* hexStr, uint8_t size);
};
}  // namespace HomieInternals

#include "Helpers.hpp"

using namespace HomieInternals;

uint8_t Helpers::rssiToPercentage(int32_t rssi) {
  uint8_t quality;
  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }

  return quality;
}

std::unique_ptr<char[]> Helpers::cloneString(const String& string) {
  size_t length = string.length();
  std::unique_ptr<char[]> copy(new char[length + 1]);
  memcpy(copy.get(), string.c_str(), length);
  copy.get()[length] = '\0';

  return copy;
}

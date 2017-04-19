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

void Helpers::stringToBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  // taken from http://stackoverflow.com/a/35236734
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);
    str = strchr(str, sep);
    if (str == NULL || *str == '\0') {
      break;
    }
    str++;
  }
}

bool Helpers::validateMacAddress(const char *mac) {
  // taken from http://stackoverflow.com/a/4792211
  int i = 0;
  int s = 0;
  while (*mac) {
    if (isxdigit(*mac)) {
      i++;
    } else if (*mac == ':' || *mac == '-') {
      if (i == 0 || i / 2 - 1 != s)
        break;
      ++s;
    } else {
       s = -1;
    }
    ++mac;
  }
  return (i == MAX_MAC_STRING_LENGTH && s == 5);
}

std::unique_ptr<char[]> Helpers::cloneString(const String& string) {
  size_t length = string.length();
  std::unique_ptr<char[]> copy(new char[length + 1]);
  memcpy(copy.get(), string.c_str(), length);
  copy.get()[length] = '\0';

  return copy;
}

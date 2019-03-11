#include "Helpers.hpp"

using namespace HomieInternals;

void Helpers::abort(const String& message) {
  Serial.begin(115200);
  Serial << message << endl;
  Serial.flush();
  ::abort();
}

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

bool Helpers::validateIP(const char* ip) {
  IPAddress test;
  return test.fromString(ip);
}

bool Helpers::validateMacAddress(const char* mac) {
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
  return (i == MAX_MAC_LENGTH * 2 && s == 5);
}

bool Helpers::validateMd5(const char* md5) {
  if (strlen(md5) != 32) return false;

  for (uint8_t i = 0; i < 32; i++) {
    char c = md5[i];
    bool valid = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    if (!valid) return false;
  }

  return true;
}

std::unique_ptr<char[]> Helpers::cloneString(const String& string) {
  size_t length = string.length();
  std::unique_ptr<char[]> copy(new char[length + 1]);
  memcpy(copy.get(), string.c_str(), length);
  copy.get()[length] = '\0';

  return copy;
}

void Helpers::ipToString(const IPAddress& ip, char * str) {
  snprintf(str, MAX_IP_STRING_LENGTH, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

void Helpers::hexStringToByteArray(const char* hexStr, uint8_t* hexArray, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    char hex[3];
    strncpy(hex, (hexStr + (i * 2)), 2);
    hex[2] = '\0';
    hexArray[i] = (uint8_t)strtol((const char*)&hex, nullptr, 16);
  }
}

void Helpers::byteArrayToHexString(const uint8_t * hexArray, char* hexStr, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    snprintf((hexStr + (i * 2)), 3, "%02x", hexArray[i]);
  }
}

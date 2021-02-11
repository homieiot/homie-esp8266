#pragma once

#include "Constants.hpp"

#if defined(HOMIE_SPIFFS)
#if defined(ESP8266)
#include <FS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif
#elif defined(HOMIE_LITTLEFS)
#include <LittleFS.h>
#endif

namespace HomieInternals {
class FS {
 public:
  FS();
  bool begin();
  bool exists(const char *filepath);
  File open(const char* path, const char* mode);
  bool remove(const char* path);

 private:
  bool _began;
};
}  // namespace HomieInternals

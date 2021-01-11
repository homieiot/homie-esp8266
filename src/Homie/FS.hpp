#pragma once

#include "Constants.hpp"

#ifdef ESP32
#ifdef HOMIE_SPIFFS
#include <SPIFFS.h>
#endif
#ifdef HOMIE_LITTLEFS
#include <LITTLEFS.h>
#endif
#elif defined(ESP8266)
#ifdef HOMIE_SPIFFS
#include "FS.h"
#elif defined(HOMIE_LITTLEFS)
#include "LittleFS.h"
#endif
#endif // ESP32

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

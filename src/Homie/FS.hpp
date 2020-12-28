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
  bool _fsBegin();
  static void doSomething();

 private:
  bool _fsBegan;
};
}  // namespace HomieInternals

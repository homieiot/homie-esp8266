#include "FS.hpp"

#include "Datatypes/Interface.hpp"

HomieInternals::FS::FS()
  : _began(false) {
}

bool HomieInternals::FS::begin() {
  if (!_began) {
#if defined(HOMIE_SPIFFS)
#if defined(ESP8266)
    _began = SPIFFS.begin();
#elif defined(ESP32)
    _began = SPIFFS.begin(true);
#endif
#elif defined(HOMIE_LITTLEFS)
    _began = LittleFS.begin();
#endif
    if (!_began) Interface::get().getLogger() << F("✖ Cannot mount filesystem") << endl;
  }
  return _began;
}

bool HomieInternals::FS::exists(const char *filepath) {
#if defined(HOMIE_SPIFFS)
  return SPIFFS.exists(filepath);
#elif defined(HOMIE_LITTLEFS)
  return LittleFS.exists(filepath);
#endif
}

File HomieInternals::FS::open(const char* path, const char* mode) {
#if defined(HOMIE_SPIFFS)
  return SPIFFS.open(path, mode);
#elif defined(HOMIE_LITTLEFS)
  return LittleFS.open(path, mode);
#endif
}

bool HomieInternals::FS::remove(const char* path) {
#if defined(HOMIE_SPIFFS)
  return SPIFFS.remove(path);
#elif defined(HOMIE_LITTLEFS)
  return LittleFS.remove(path);
#endif
}

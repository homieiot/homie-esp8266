#include "FS.hpp"

#include "Datatypes/Interface.hpp"

HomieInternals::FS::FS()
  : _began(false) {
}

bool HomieInternals::FS::begin() {
  if (!_began) {
#ifdef ESP32
#ifdef HOMIE_SPIFFS
    _began = SPIFFS.begin(true);
#elif defined(HOMIE_LITTLEFS)
    _began = LittleFS.begin();
#endif
#elif defined(ESP8266)
#ifdef HOMIE_SPIFFS
    _began = SPIFFS.begin();
#elif defined(HOMIE_LITTLEFS)
    _began = LittleFS.begin();
#endif
#endif
    if (!_began) Interface::get().getLogger() << F("✖ Cannot mount filesystem") << endl;
  }

  return _began;
}

bool HomieInternals::FS::exists(const char *filepath) {
  return SPIFFS.exists(filepath);
}

File HomieInternals::FS::open(const char* path, const char* mode) {
  return SPIFFS.open(path, mode);
}

bool HomieInternals::FS::remove(const char* path) {
  return SPIFFS.remove(path);
}

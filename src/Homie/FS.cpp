#include "FS.hpp"
#include "Datatypes/Interface.hpp"

HomieInternals::FS::FS()
  : _fsBegan(false) {
}

bool HomieInternals::FS::_fsBegin() {
  if (!_fsBegan) {
#ifdef ESP32
#ifdef HOMIE_SPIFFS
    _fsBegan = SPIFFS.begin(true);
#elif defined(HOMIE_LITTLEFS)
    _fsBegan = LittleFS.begin();
#endif
#elif defined(ESP8266)
#ifdef HOMIE_SPIFFS
    _fsBegan = SPIFFS.begin();
#elif defined(HOMIE_LITTLEFS)
    _fsBegan = LittleFS.begin();
#endif
#endif
    if (!_fsBegan) Interface::get().getLogger() << F("✖ Cannot mount filesystem") << endl;
  }

  return _fsBegan;
}

void HomieInternals::FS::doSomething() {
  bool lala = false;
}

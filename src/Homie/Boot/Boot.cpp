#include "Boot.hpp"
#include "../Config.hpp"

using namespace HomieInternals;

Boot::Boot(const char* name)
: _interface(nullptr)
, _name(name) {
}

void Boot::attachInterface(Interface* interface) {
  _interface = interface;
}

void Boot::setup() {
  if (_interface->led.enabled) {
    pinMode(_interface->led.pin, OUTPUT);
    digitalWrite(_interface->led.pin, !_interface->led.on);
  }

  WiFi.persistent(true);  // Persist data on SDK as it seems Wi-Fi connection is faster

  _interface->logger->log(F("** Booting into "));
  _interface->logger->log(_name);
  _interface->logger->logln(F(" mode **"));
}

void Boot::loop() {
}

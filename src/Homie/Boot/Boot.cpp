#include "Boot.hpp"

using namespace HomieInternals;

Boot::Boot(const char* name)
: _name(name) {
}


void Boot::setup() {
  if (Interface::get().led.enabled) {
    pinMode(Interface::get().led.pin, OUTPUT);
    digitalWrite(Interface::get().led.pin, !Interface::get().led.on);
  }

  WiFi.persistent(true);  // Persist data on SDK as it seems Wi-Fi connection is faster

  Interface::get().logger->print(F("** Booting into "));
  Interface::get().logger->print(_name);
  Interface::get().logger->println(F(" mode **"));
}

void Boot::loop() {
}

void Boot::prepareToSleep() {
}

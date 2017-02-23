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

  Interface::get().getLogger() << F("💡 Firmware ") << Interface::get().firmware.name << F(" (") << Interface::get().firmware.version << F(")") << endl;
  Interface::get().getLogger() << F("🔌 Booting into ") << _name << F(" mode 🔌") << endl;
}

void Boot::loop() {
}

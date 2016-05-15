#include "Boot.hpp"
#include "../Config.hpp"

using namespace HomieInternals;

Boot::Boot(const char* name)
: _interface(nullptr)
, _name(name)
{
}

void Boot::attachInterface(Interface* interface) {
  _interface = interface;
}

void Boot::setup() {
  if (this->_interface->led.enabled) {
    pinMode(this->_interface->led.pin, OUTPUT);
    digitalWrite(this->_interface->led.pin, !this->_interface->led.on);
  }

  WiFi.persistent(false); // Don't persist data on EEPROM since this is handled by Homie
  WiFi.disconnect(); // Reset network state

  WiFi.hostname(this->_interface->config->get().deviceId);

  this->_interface->logger->log(F("** Booting into "));
  this->_interface->logger->log(this->_name);
  this->_interface->logger->logln(F(" mode **"));
}

void Boot::loop() {
}

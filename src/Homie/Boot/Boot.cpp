#include "Boot.hpp"

using namespace HomieInternals;

Boot::Boot(Interface* interface, const char* name)
: _interface(interface)
, _name(name)
{
}

void Boot::setup() {
  if (this->_interface->led.enable) {
    pinMode(this->_interface->led.pin, OUTPUT);
    digitalWrite(this->_interface->led.pin, !this->_interface->led.on);
  }

  WiFi.persistent(false); // Don't persist data on EEPROM since this is handled by Homie
  WiFi.disconnect(); // Reset network state

  char hostname[MAX_LENGTH_WIFI_SSID] = "";
  strcat(hostname, this->_interface->brand);
  strcat(hostname, "-");
  strcat(hostname, Helpers.getDeviceId());

  WiFi.hostname(hostname);

  Logger.logln();
  Logger.log("** Booting in ");
  Logger.log(this->_name);
  Logger.logln(" mode **");
}

void Boot::loop() {
}

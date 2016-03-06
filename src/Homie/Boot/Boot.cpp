#include "Boot.hpp"

using namespace HomieInternals;

Boot::Boot(SharedInterface* shared_interface, const char* name)
: _shared_interface(shared_interface)
, _name(name)

{
}

void Boot::setup() {
  if (this->_shared_interface->useBuiltInLed) {
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, HIGH); // low active
  }

  WiFi.persistent(false); // Don't persist data on EEPROM since this is handled by Homie
  WiFi.disconnect(); // Reset network state

  char hostname[CONFIG_MAX_LENGTH_WIFI_SSID] = "";
  strcat(hostname, this->_shared_interface->brand);
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

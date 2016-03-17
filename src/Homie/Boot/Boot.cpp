#include "Boot.hpp"

using namespace HomieInternals;

Boot::Boot(SharedInterface* sharedInterface, const char* name)
: _sharedInterface(sharedInterface)
, _name(name)

{
}

void Boot::setup() {
  if (this->_sharedInterface->useBuiltInLed) {
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, HIGH); // low active
  }

  WiFi.persistent(false); // Don't persist data on EEPROM since this is handled by Homie
  WiFi.disconnect(); // Reset network state

  char hostname[CONFIG_MAX_LENGTH_WIFI_SSID] = "";
  strcat(hostname, this->_sharedInterface->brand);
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

#include "Boot.hpp"

using namespace HomieInternals;

Boot::Boot(const char* name)
: _name(name)
{
}

void Boot::setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH); // low active

  WiFi.persistent(false); // Don't persist data on EEPROM since this is handled by Homie
  WiFi.disconnect(); // Reset network state

  char hostname[14 + 1] = "Homie-";
  strcat(hostname, Helpers.getDeviceId());

  WiFi.hostname(hostname);

  Logger.logln();
  Logger.log("** Booting in ");
  Logger.log(this->_name);
  Logger.logln(" mode **");
}

void Boot::loop() {
}

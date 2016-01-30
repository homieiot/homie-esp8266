#include "Boot.hpp"

using namespace HomieInternals;

Boot::Boot(const char* name)
: _name(name)
{
}

void Boot::setup() {
  pinMode(PIN_RESET, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH); // low active

  Serial.begin(BAUD_RATE);
  WiFi.persistent(false); // Don't persist data on EEPROM since this is handled by Homie
  WiFi.disconnect(); // Reset network state

  String hostname = String("Homie-");
  hostname += Helpers::getDeviceId();

  WiFi.hostname(hostname);

  Logger.logln();
  Logger.log("** Booting in ");
  Logger.log(this->_name);
  Logger.logln(" mode **");
}

void Boot::loop() {
}

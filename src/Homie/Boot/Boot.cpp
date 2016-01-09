#include "Boot.hpp"

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

  Serial.println();
  Serial.print("Booting in ");
  Serial.print(this->_name);
  Serial.println(" mode");
}

void Boot::loop() {
}

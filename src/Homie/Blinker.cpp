#include "Blinker.hpp"

using namespace HomieInternals;

BlinkerClass::BlinkerClass()
: _lastBlinkPace(0)
// , _interface(nullptr) causes exception ???
{
}

void BlinkerClass::attachInterface(Interface* interface) {
  this->_interface = interface;
}

void BlinkerClass::start(float blinkPace) {
  Serial.println("here1");
  if (this->_lastBlinkPace != blinkPace) {
    Serial.println("here2");
    Serial.println(this->_interface->led.pin);
    Serial.println("here3");
    this->_ticker.attach(blinkPace, this->_tick, this->_interface->led.pin);
    this->_lastBlinkPace = blinkPace;
    Serial.println("here4");
  }
}

void BlinkerClass::stop() {
  if (this->_lastBlinkPace != 0) {
    this->_ticker.detach();
    this->_lastBlinkPace = 0;
    digitalWrite(this->_interface->led.pin, !this->_interface->led.on);
  }
}

void BlinkerClass::_tick(unsigned char pin) {
  digitalWrite(pin, !digitalRead(pin));
}

BlinkerClass HomieInternals::Blinker;

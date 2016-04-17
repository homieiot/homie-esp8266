#include "Blinker.hpp"

using namespace HomieInternals;

BlinkerClass::BlinkerClass()
:  _interface(nullptr)
, _lastBlinkPace(0)
{
}

void BlinkerClass::attachInterface(Interface* interface) {
  this->_interface = interface;
}

void BlinkerClass::start(float blinkPace) {
  if (this->_lastBlinkPace != blinkPace) {
    this->_ticker.attach(blinkPace, this->_tick, this->_interface->led.pin);
    this->_lastBlinkPace = blinkPace;
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

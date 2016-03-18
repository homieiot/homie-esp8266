#include "Blinker.hpp"

using namespace HomieInternals;

BlinkerClass::BlinkerClass()
: _lastBlinkPace(0)
{
}

void BlinkerClass::attachSharedInterface(SharedInterface* sharedInterface) {
  this->_sharedInterface = sharedInterface;
}

void BlinkerClass::start(float blinkPace) {
  if (this->_lastBlinkPace != blinkPace) {
    this->_ticker.attach(blinkPace, this->_tick, this->_sharedInterface->ledPin);
    this->_lastBlinkPace = blinkPace;
  }
}

void BlinkerClass::stop() {
  if (this->_lastBlinkPace != 0) {
    this->_ticker.detach();
    this->_lastBlinkPace = 0;
    digitalWrite(this->_sharedInterface->ledPin, HIGH);
  }
}

void BlinkerClass::_tick(byte pin) {
  digitalWrite(pin, !digitalRead(pin));
}

BlinkerClass HomieInternals::Blinker;

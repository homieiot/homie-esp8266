#include "Blinker.hpp"

using namespace HomieInternals;

BlinkerClass::BlinkerClass()
: _lastBlinkPace(0)
{
}

void BlinkerClass::start(float blinkPace) {
  if (this->_lastBlinkPace != blinkPace) {
    this->_ticker.attach(blinkPace, this->_tick);
    this->_lastBlinkPace = blinkPace;
  }
}

void BlinkerClass::stop() {
  if (this->_lastBlinkPace != 0) {
    this->_ticker.detach();
    this->_lastBlinkPace = 0;
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void BlinkerClass::_tick() {
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
}

BlinkerClass HomieInternals::Blinker;

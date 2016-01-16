#include "Blinker.hpp"

using namespace HomieInternals;

BlinkerClass::BlinkerClass()
: _last_blink_pace(0)
{
}

void BlinkerClass::start(float blink_pace) {
  if (this->_last_blink_pace != blink_pace) {
    this->_ticker.attach(blink_pace, this->_tick);
    this->_last_blink_pace = blink_pace;
  }
}

void BlinkerClass::stop() {
  if (this->_last_blink_pace != 0) {
    this->_ticker.detach();
    this->_last_blink_pace = 0;
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void BlinkerClass::_tick() {
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
}

BlinkerClass HomieInternals::Blinker;

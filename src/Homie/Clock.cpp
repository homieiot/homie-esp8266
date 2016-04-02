#include "Clock.hpp"

using namespace HomieInternals;

ClockClass::ClockClass()
: _seconds(0)
, _lastTick(0)
{
}

void ClockClass::tick() {
  unsigned long now = millis();
  this->_seconds += (now - this->_lastTick) / 1000UL;
  this->_lastTick = now;
}

unsigned long ClockClass::getSeconds() {
  return this->_seconds;
}

ClockClass HomieInternals::Clock;

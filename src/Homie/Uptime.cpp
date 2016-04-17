#include "Uptime.hpp"

using namespace HomieInternals;

Uptime::Uptime()
: _seconds(0)
, _lastTick(0)
{
}

void Uptime::update() {
  unsigned long now = millis();
  this->_seconds += (now - this->_lastTick) / 1000UL;
  this->_lastTick = now;
}

unsigned long Uptime::getSeconds() {
  return this->_seconds;
}

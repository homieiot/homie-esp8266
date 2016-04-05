#include "Uptime.hpp"

using namespace HomieInternals;

UptimeClass::UptimeClass()
: _seconds(0)
, _lastTick(0)
{
}

void UptimeClass::update() {
  unsigned long now = millis();
  this->_seconds += (now - this->_lastTick) / 1000UL;
  this->_lastTick = now;
}

unsigned long UptimeClass::getSeconds() {
  return this->_seconds;
}

UptimeClass HomieInternals::Uptime;

#include "Uptime.hpp"

using namespace HomieInternals;

Uptime::Uptime()
: _seconds(0)
, _lastTick(0)
, _rolloverCount(0)   {
}

void Uptime::update() {
  uint32_t now = millis();
  
  if (now < _lastTick) {
    _rolloverCount++;
  }
  
  _lastTick = now;
  _seconds =  (0xFFFFFFFF / 1000 ) * _rolloverCount + (_lastTick / 1000);
}

uint32_t Uptime::getSeconds() const {
  return _seconds;
}

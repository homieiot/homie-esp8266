#include "Uptime.hpp"

using namespace HomieInternals;

Uptime::Uptime()
: _seconds(0)
, _lastTick(0) {
}

void Uptime::update() {
  uint32_t now = millis();
  _seconds += (now - _lastTick) / 1000UL;
  _lastTick = now;
}

uint64_t Uptime::getSeconds() const {
  return _seconds;
}

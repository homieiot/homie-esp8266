#include "Uptime.hpp"

using namespace HomieInternals;

Uptime::Uptime()
: _milliseconds(0)
, _lastTick(0) {
}

void Uptime::update() {
  uint32_t now = millis();
  _milliseconds += (now - _lastTick);
  _lastTick = now;
}

uint64_t Uptime::getSeconds() const {
  return (_milliseconds / 1000ULL);
}

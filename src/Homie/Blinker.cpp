#include "Blinker.hpp"

using namespace HomieInternals;

Blinker::Blinker()
: _interface(nullptr)
, _lastBlinkPace(0)
{
}

void Blinker::attachInterface(Interface* interface) {
  _interface = interface;
}

void Blinker::start(float blinkPace) {
  if (_lastBlinkPace != blinkPace) {
    _ticker.attach(blinkPace, _tick, _interface->led.pin);
    _lastBlinkPace = blinkPace;
  }
}

void Blinker::stop() {
  if (_lastBlinkPace != 0) {
    _ticker.detach();
    _lastBlinkPace = 0;
    digitalWrite(_interface->led.pin, !_interface->led.on);
  }
}

void Blinker::_tick(uint8_t pin) {
  digitalWrite(pin, !digitalRead(pin));
}

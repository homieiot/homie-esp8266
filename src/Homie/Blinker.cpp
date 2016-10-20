#include "Blinker.hpp"

using namespace HomieInternals;

Blinker::Blinker()
: _lastBlinkPace(0) {
}

void Blinker::start(float blinkPace) {
  if (_lastBlinkPace != blinkPace) {
    _ticker.attach(blinkPace, _tick, Interface::get().led.pin);
    _lastBlinkPace = blinkPace;
  }
}

void Blinker::stop() {
  if (_lastBlinkPace != 0) {
    _ticker.detach();
    _lastBlinkPace = 0;
    digitalWrite(Interface::get().led.pin, !Interface::get().led.on);
  }
}

void Blinker::_tick(uint8_t pin) {
  digitalWrite(pin, !digitalRead(pin));
}

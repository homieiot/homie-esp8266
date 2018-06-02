#include "Timer.hpp"

using namespace HomieInternals;

Timer::Timer()
: _initialTime(0)
, _interval(0)
, _tickAtBeginning(false)
, _active(true) {
}

void Timer::setInterval(uint32_t interval, bool tickAtBeginning) {
  _interval = interval;
  _tickAtBeginning = tickAtBeginning;

  this->reset();
}

uint32_t HomieInternals::Timer::getInterval() {
  return _interval;
}

bool Timer::check() const {
  if (!_active) return false;

  if (_tickAtBeginning && _initialTime == 0) return true;
  if (millis() - _initialTime >= _interval) return true;

  return false;
}

void Timer::reset() {
  if (_tickAtBeginning) {
    _initialTime = 0;
  } else {
    this->tick();
  }
}

void Timer::tick() {
  _initialTime = millis();
}

void Timer::activate() {
  _active = true;
}

void Timer::deactivate() {
  _active = false;
  reset();
}

bool Timer::isActive() const {
  return _active;
}

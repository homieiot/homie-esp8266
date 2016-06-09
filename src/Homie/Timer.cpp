#include "Timer.hpp"

using namespace HomieInternals;

Timer::Timer()
: _initialTime(0)
, _interval(0)
, _tickAtBeginning(false)
{
}

void Timer::setInterval(uint32_t interval, bool tickAtBeginning) {
  _interval = interval;
  _tickAtBeginning = tickAtBeginning;

  if (!tickAtBeginning) _initialTime = millis();
}

bool Timer::check() const {
  if (_tickAtBeginning && _initialTime == 0) return true;
  if (millis() - _initialTime >= _interval) return true;

  return false;
}

void Timer::reset() {
  _initialTime = 0;
}

void Timer::tick() {
  _initialTime = millis();
}

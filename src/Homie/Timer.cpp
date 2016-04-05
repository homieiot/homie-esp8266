#include "Timer.hpp"

using namespace HomieInternals;

Timer::Timer()
: _initialTime(0)
, _interval(0)
, _tickAtBeginning(false)
{
}

void Timer::setInterval(unsigned long interval, bool tickAtBeginning) {
  this->_interval = interval;
  this->_tickAtBeginning = tickAtBeginning;

  if (!tickAtBeginning) this->_initialTime = millis();
}

bool Timer::check() {
  if (this->_tickAtBeginning && this->_initialTime == 0) return true;
  if (millis() - this->_initialTime >= this->_interval) return true;

  return false;
}

void Timer::reset() {
  this->_initialTime = 0;
}

void Timer::tick() {
  this->_initialTime = millis();
}

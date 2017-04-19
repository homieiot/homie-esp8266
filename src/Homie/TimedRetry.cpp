#include "TimedRetry.hpp"
#include "Datatypes/Interface.hpp"

using namespace HomieInternals;

TimedRetry::TimedRetry(uint32_t stepInterval, uint32_t maxInterval)
: _currentStep(0)
, _stepInterval(stepInterval)
, _maxInterval(maxInterval)
, _timer(Timer()) {
  _timer.deactivate();
}

void TimedRetry::activate() {
  if (!_timer.isActive()) {
    _timer.setInterval(_stepInterval, false);
    _timer.activate();
    _currentStep = 1;
  }
}

bool TimedRetry::check() {
  if (_timer.check()) {
    long nextInterval = (_currentStep*2)*_stepInterval;
    if (nextInterval <= _maxInterval) {
      _currentStep = _currentStep*2;
    } else {
      nextInterval = _maxInterval;
    }
    // setInterval does tick()
    _timer.setInterval(nextInterval, false);
    Interface::get().getLogger() << F("Retrying (") << nextInterval << F("ms)") << F("...") << endl;
    return true;

  } else {
    return false;
  }
}

void TimedRetry::deactivate() {
  _timer.deactivate();
  _timer.reset();
}

bool TimedRetry::isActive() const {
  return _timer.isActive();
}

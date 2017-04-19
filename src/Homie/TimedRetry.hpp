#pragma once

#include "Timer.hpp"

namespace HomieInternals {
class TimedRetry {
 public:
  TimedRetry(uint32_t stepInterval, uint32_t maxInterval);
  void activate();
  bool check();
  void deactivate();
  bool isActive() const;

 private:
  uint32_t _currentStep;
  uint32_t _stepInterval;
  uint32_t _maxInterval;
  Timer _timer;
};
}  // namespace HomieInternals

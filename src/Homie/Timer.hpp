#pragma once

#include "Arduino.h"

namespace HomieInternals {
class Timer {
 public:
  Timer();
  void setInterval(uint32_t interval, bool tickAtBeginning = true);
  uint32_t getInterval();
  bool check() const;
  void tick();
  void reset();
  void activate();
  void deactivate();
  bool isActive() const;

 private:
  uint32_t _initialTime;
  uint32_t _interval;
  bool _tickAtBeginning;
  bool _active;
};
}  // namespace HomieInternals

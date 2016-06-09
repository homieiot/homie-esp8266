#pragma once

#include "Arduino.h"

namespace HomieInternals {
  class Timer {
    public:
      Timer();
      void setInterval(uint32_t interval, bool tickAtBeginning = true);
      bool check() const;
      void tick();
      void reset();

    private:
      uint32_t _initialTime;
      uint32_t _interval;
      bool _tickAtBeginning;
  };
}

#pragma once

#include "Arduino.h"

namespace HomieInternals {
  class Timer {
    public:
      Timer();
      void setInterval(unsigned long interval, bool tickAtBeginning = true);
      bool check();
      void tick();
      void reset();

    private:
      unsigned long _initialTime;
      unsigned long _interval;
      bool _tickAtBeginning;
  };
}

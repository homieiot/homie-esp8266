#pragma once

#include "Arduino.h"

namespace HomieInternals {
  class ClockClass {
    public:
      ClockClass();
      void tick();
      unsigned long getSeconds();

    private:
      unsigned long _seconds;
      unsigned long _lastTick;
  };

  extern ClockClass Clock;
}

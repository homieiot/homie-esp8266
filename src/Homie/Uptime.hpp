#pragma once

#include "Arduino.h"

namespace HomieInternals {
  class Uptime {
    public:
      Uptime();
      void update();
      unsigned long getSeconds();

    private:
      unsigned long _seconds;
      unsigned long _lastTick;
  };
}

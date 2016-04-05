#pragma once

#include "Arduino.h"

namespace HomieInternals {
  class UptimeClass {
    public:
      UptimeClass();
      void update();
      unsigned long getSeconds();

    private:
      unsigned long _seconds;
      unsigned long _lastTick;
  };

  extern UptimeClass Uptime;
}

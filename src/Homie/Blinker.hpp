#pragma once

#include <Arduino.h>
#include <Ticker.h>

namespace HomieInternals {
  class BlinkerClass {
    public:
      BlinkerClass();
      void start(float blink_pace);
      void stop();

    private:
      Ticker _ticker;
      float _last_blink_pace;

      static void _tick();
  };

  extern BlinkerClass Blinker;
}

#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include "Datatypes/SharedInterface.hpp"

namespace HomieInternals {
  class BlinkerClass {
    public:
      BlinkerClass();
      void attachSharedInterface(SharedInterface* sharedInterface);
      void start(float blink_pace);
      void stop();

    private:
      SharedInterface* _sharedInterface;
      Ticker _ticker;
      float _lastBlinkPace;

      static void _tick(byte pin);
  };

  extern BlinkerClass Blinker;
}

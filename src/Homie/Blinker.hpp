#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include "Datatypes/Interface.hpp"

namespace HomieInternals {
  class BlinkerClass {
    public:
      BlinkerClass();
      void attachInterface(Interface* interface);
      void start(float blink_pace);
      void stop();

    private:
      Interface* _interface;
      Ticker _ticker;
      float _lastBlinkPace;

      static void _tick(byte pin);
  };

  extern BlinkerClass Blinker;
}

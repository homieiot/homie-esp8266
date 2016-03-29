#pragma once

#include <Ticker.h>
#include "Datatypes/Interface.hpp"

namespace HomieInternals {
  class BlinkerClass {
    public:
      BlinkerClass();
      void attachInterface(Interface* interface);
      void start(float blinkPace);
      void stop();

    private:
      Interface* _interface;
      Ticker _ticker;
      float _lastBlinkPace;

      static void _tick(unsigned char pin);
  };

  extern BlinkerClass Blinker;
}

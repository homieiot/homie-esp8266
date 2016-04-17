#pragma once

#include <Ticker.h>
#include "Datatypes/Interface.hpp"

namespace HomieInternals {
  class Blinker {
    public:
      Blinker();
      void attachInterface(Interface* interface);
      void start(float blinkPace);
      void stop();

    private:
      Interface* _interface;
      Ticker _ticker;
      float _lastBlinkPace;

      static void _tick(unsigned char pin);
  };
}

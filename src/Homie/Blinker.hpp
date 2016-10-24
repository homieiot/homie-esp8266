#pragma once

#include <Ticker.h>
#include "Datatypes/Interface.hpp"

namespace HomieInternals {
class Blinker {
 public:
  Blinker();
  void start(float blinkPace);
  void stop();

 private:
  Ticker _ticker;
  float _lastBlinkPace;

  static void _tick(uint8_t pin);
};
}  // namespace HomieInternals

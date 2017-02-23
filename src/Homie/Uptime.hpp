#pragma once

#include "Arduino.h"

namespace HomieInternals {
class Uptime {
 public:
  Uptime();
  void update();
  uint32_t getSeconds() const;

 private:
  uint32_t _seconds;
  uint32_t _lastTick;
  uint32_t _rolloverCount;
};
}  // namespace HomieInternals

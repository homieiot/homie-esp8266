#pragma once

#include "Arduino.h"

#include <Ticker.h>
#include <Bounce2.h>
#include "../../StreamingOperator.hpp"
#include "../Datatypes/Interface.hpp"

namespace HomieInternals {
class ResetHandler {
 public:
  static void attach();

 private:
  // Disable creating an instance of this object
  ResetHandler() {}

  static Ticker _resetTicker;
  static bool _sentReset;
  static bool _sentRestart;

  static void _tick();
  static void _handleReboot();
  static void _handleReset();
};
}  // namespace HomieInternals

#pragma once

#include "Arduino.h"

#include <Ticker.h>
#include <Bounce2.h>
#include "../../StreamingOperator.hpp"
#include "../Datatypes/Interface.hpp"

#if HOMIE_CONFIG
namespace HomieInternals {
class ResetHandler {
 public:
  static void Attach();

 private:
  // Disallow creating an instance of this object
  ResetHandler() {}
  static Ticker _resetBTNTicker;
  static Bounce _resetBTNDebouncer;
  static void _tick();
  static Ticker _resetTicker;
  static bool _sentReset;
  static void _handleReset();
};
}  // namespace HomieInternals
#endif

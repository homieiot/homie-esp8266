#pragma once

#include "Arduino.h"

#include <Ticker.h>
#include <Bounce2.h>
#include "../../StreamingOperator.hpp"
#include "../Datatypes/Interface.hpp"

namespace HomieInternals {
  class ResetButton {
  public:
    static void Attach();
    static bool _flaggedForReset;

  private:
    // Disallow creating an instance of this object
    ResetButton() {}
    static Ticker _resetTicker;
    static Bounce _resetDebouncer;
    static void _handleReset();
  };
}  // namespace HomieInternals

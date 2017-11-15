#pragma once

#include "Arduino.h"

#include <Bounce2.h>
#include "../../StreamingOperator.hpp"
#include "../Datatypes/Interface.hpp"

namespace HomieInternals {
  class ResetButton {
  protected:
    explicit ResetButton();
    void Attach();
    bool _flaggedForReset;
    Bounce _resetDebouncer;
    virtual void _handleReset();
  };
}  // namespace HomieInternals

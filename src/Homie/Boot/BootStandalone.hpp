#pragma once

#include "Arduino.h"

#include "Boot.hpp"
#include "../../StreamingOperator.hpp"
#include "../Utils/ResetButton.hpp"

namespace HomieInternals {
  class BootStandalone : public Boot, public ResetButton {
  public:
    BootStandalone();
    ~BootStandalone();
    void setup();
    void loop();
  };
}  // namespace HomieInternals

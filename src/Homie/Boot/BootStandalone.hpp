#pragma once

#include "Arduino.h"

#include "Boot.hpp"
#include "../../StreamingOperator.hpp"
#include "../Utils/ResetHandler.hpp"

namespace HomieInternals {
class BootStandalone : public Boot {
 public:
  BootStandalone();
  ~BootStandalone();
  void setup();
  void loop();
};
}  // namespace HomieInternals

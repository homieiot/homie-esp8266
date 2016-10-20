#pragma once

#include "Arduino.h"

#include <Bounce2.h>
#include "../../StreamingOperator.hpp"
#include "Boot.hpp"

namespace HomieInternals {
class BootStandalone : public Boot {
 public:
  BootStandalone();
  ~BootStandalone();
  void setup();
  void loop();

 private:
  bool _flaggedForConfig;
  Bounce _resetDebouncer;

  void _handleReset();
};
}  // namespace HomieInternals

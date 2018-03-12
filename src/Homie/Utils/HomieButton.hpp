#pragma once

#include "Arduino.h"

#include <Ticker.h>
#include <OneButton.h>
#include "../../StreamingOperator.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Datatypes/Callbacks.hpp"

namespace HomieInternals {
class HomieButton{
 public:
  static void attach();
  static void setClickHandler(CallbackFunction& function);

 private:
    // Disable creating an instance of this object
  HomieButton() {}

  static Ticker _homieButtonTicker;
  static OneButton* _homieButton;
  static CallbackFunction _userClickFunc;

  static void _tick();
  static void _clickFunc();
  static void _doubleClickFunc();
  static void _longPressStartFunc();
};
} //  namespace HomieInternals

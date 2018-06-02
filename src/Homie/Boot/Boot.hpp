#pragma once

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include "../../StreamingOperator.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Utils/Helpers.hpp"

namespace HomieInternals {
class Boot {
 public:
  explicit Boot(const char* name);
  virtual void setup();
  virtual void loop();

 protected:
  const char* _name;
};
}  // namespace HomieInternals

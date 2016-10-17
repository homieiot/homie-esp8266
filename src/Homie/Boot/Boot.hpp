#pragma once

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include "../Datatypes/Interface.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Logger.hpp"
#include "../Utils/Helpers.hpp"

namespace HomieInternals {
class Boot {
 public:
  explicit Boot(const char* name);
  virtual void setup();
  virtual void loop();
  virtual void prepareToSleep();

  void attachInterface(Interface* interface);

 protected:
  Interface* _interface;
  const char* _name;
};
}  // namespace HomieInternals

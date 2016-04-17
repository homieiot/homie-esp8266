#pragma once

#include "Arduino.h"

namespace HomieInternals {
  class LoggerClass {
    public:
      LoggerClass();
      void setLogging(bool enable);
      bool isEnabled();
      template <typename T> void log(T value) {
        if (this->_loggingEnabled) {
          Serial.print(value);
        }
      }
      template <typename T> void logln(T value) {
        if (this->_loggingEnabled) {
          Serial.println(value);
        }
      }
      void logln();

    private:
      bool _loggingEnabled;
  };

  extern LoggerClass Logger;
}

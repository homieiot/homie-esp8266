#pragma once

#include "Arduino.h"

namespace HomieInternals {
  class Logger {
    public:
      Logger();
      void setPrinter(Print* printer);
      void setLogging(bool enable);
      bool isEnabled() const;
      template <typename T> void log(T value) const {
        if (this->_loggingEnabled) {
          _printer->print(value);
        }
      }
      template <typename T> void logln(T value) const {
        if (this->_loggingEnabled) {
          _printer->println(value);
        }
      }
      void logln() const;

    private:
      bool _loggingEnabled;
      Print* _printer;
  };
}

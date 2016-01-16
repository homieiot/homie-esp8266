#pragma once

#include <Arduino.h>

namespace HomieInternals {
  class LoggerClass {
    public:
      LoggerClass();
      void setLogging(bool enable);
      void log(const char* text) {
        this->log(String(text));
      }
      void log(String text);
      void logln(const char* text = "") {
        this->logln(String(text));
      }
      void logln(String text);

    private:
      bool _logging_enabled;
  };

  extern LoggerClass Logger;
}

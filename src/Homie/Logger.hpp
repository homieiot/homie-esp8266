#pragma once

#include <Arduino.h>

namespace HomieInternals {
  class LoggerClass {
    public:
      LoggerClass();
      void setLogging(bool enable);
      void log(String text) {
        this->log(text.c_str());
      }
      void log(const char* text);
      void logln(String text) {
        this->logln(text.c_str());
      }
      void logln(const char* text = "");

    private:
      bool _logging_enabled;
  };

  extern LoggerClass Logger;
}

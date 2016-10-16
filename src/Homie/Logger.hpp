#pragma once

#include "Arduino.h"

namespace HomieInternals {
class Logger : public Print {
 public:
  Logger();
  void setPrinter(Print* printer);
  void setLogging(bool enable);
  virtual size_t write(uint8_t character);
  virtual size_t write(const uint8_t* buffer, size_t size);

 private:
  bool _loggingEnabled;
  Print* _printer;
};
}  // namespace HomieInternals

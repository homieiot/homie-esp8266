#include "Logger.hpp"

using namespace HomieInternals;

LoggerClass::LoggerClass()
: _logging_enabled(true)
{
}

bool LoggerClass::isEnabled() {
  return this->_logging_enabled;
}

void LoggerClass::setLogging(bool enable) {
  this->_logging_enabled = enable;
}

void LoggerClass::log(const char* text) {
  if (this->_logging_enabled) {
    Serial.print(text);
  }
}

void LoggerClass::logln(const char* text) {
  if (this->_logging_enabled) {
    Serial.println(text);
  }
}

LoggerClass HomieInternals::Logger;

#include "Logger.hpp"

using namespace HomieInternals;

LoggerClass::LoggerClass()
: _loggingEnabled(true)
{
}

bool LoggerClass::isEnabled() {
  return this->_loggingEnabled;
}

void LoggerClass::setLogging(bool enable) {
  this->_loggingEnabled = enable;
}

void LoggerClass::log(const char* text) {
  if (this->_loggingEnabled) {
    Serial.print(text);
  }
}

void LoggerClass::logln(const char* text) {
  if (this->_loggingEnabled) {
    Serial.println(text);
  }
}

LoggerClass HomieInternals::Logger;

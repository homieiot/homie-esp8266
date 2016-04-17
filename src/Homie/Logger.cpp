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

void LoggerClass::logln() {
  if (this->_loggingEnabled) {
    Serial.println();
  }
}

LoggerClass HomieInternals::Logger;

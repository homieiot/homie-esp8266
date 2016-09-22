#include "Logger.hpp"

using namespace HomieInternals;

Logger::Logger()
: _loggingEnabled(true)
{
}

bool Logger::isEnabled() {
  return this->_loggingEnabled;
}

void Logger::setLogging(bool enable) {
  this->_loggingEnabled = enable;
}

void Logger::logln() {
  if (this->_loggingEnabled) {
    Serial.println();
  }
}

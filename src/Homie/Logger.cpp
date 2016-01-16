#include "Logger.hpp"

LoggerClass::LoggerClass()
: _logging_enabled(true)
{
}

void LoggerClass::setLogging(bool enable) {
  this->_logging_enabled = enable;
}

void LoggerClass::log(String text) {
  if (this->_logging_enabled) {
    Serial.print(text);
  }
}

void LoggerClass::logln(String text) {
  if (this->_logging_enabled) {
    Serial.println(text);
  }
}

LoggerClass Logger;

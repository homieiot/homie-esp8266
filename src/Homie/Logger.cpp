#include "Logger.hpp"

using namespace HomieInternals;

Logger::Logger()
: _loggingEnabled(true)
, _printer(&Serial)
{
}

bool Logger::isEnabled() const {
  return _loggingEnabled;
}

void Logger::setLogging(bool enable) {
  _loggingEnabled = enable;
}

void Logger::setPrinter(Print* printer) {
  _printer = printer;
}

void Logger::logln() const {
  if (_loggingEnabled) {
    _printer->println();
  }
}

void Logger::flush() const {
  if (_loggingEnabled && _printer == &Serial) {
    static_cast<Stream*>(_printer)->flush();
  }
}

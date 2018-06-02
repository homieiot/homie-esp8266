#include "Logger.hpp"

using namespace HomieInternals;

Logger::Logger()
: _loggingEnabled(true)
, _printer(&Serial) {
}

void Logger::setLogging(bool enable) {
  _loggingEnabled = enable;
}

void Logger::setPrinter(Print* printer) {
  _printer = printer;
}

size_t Logger::write(uint8_t character) {
  if (_loggingEnabled) return _printer->write(character);
  return 0;
}

size_t Logger::write(const uint8_t* buffer, size_t size) {
  if (_loggingEnabled) return _printer->write(buffer, size);
  return 0;
}

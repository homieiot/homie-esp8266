#pragma once

namespace HomieInternals {
struct Result {
  bool success;
  String message;
};
struct ValidationResult {
  bool valid;
  String reason;
};
struct ValidationResultOBJ : ValidationResult {
  JsonObject* config;
};
}// namespace HomieInternals

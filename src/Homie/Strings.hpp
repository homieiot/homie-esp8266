#pragma once

namespace HomieInternals {
  // config mode

  const char PROGMEM_CONFIG_APPLICATION_JSON[] PROGMEM = "application/json";
  const char PROGMEM_CONFIG_JSON_SUCCESS[] PROGMEM = "{\"success\":true}";
  const char PROGMEM_CONFIG_JSON_FAILURE_BEGINNING[] PROGMEM = "{\"success\":false,\"error\":\"";
  const char PROGMEM_CONFIG_JSON_FAILURE_END[] PROGMEM = "\"}";

  const char PROGMEM_CONFIG_SPIFFS_NOT_FOUND[] PROGMEM = "✖ Cannot mount filesystem";
  const char PROGMEM_CONFIG_FILE_NOT_FOUND[] PROGMEM = "✖ Cannot open config file";
  const char PROGMEM_CONFIG_FILE_TOO_BIG[] PROGMEM = "✖ Config file too big";

} // namespace HomieInternals

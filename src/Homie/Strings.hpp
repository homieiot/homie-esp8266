#pragma once

namespace HomieInternals {
  // config mode

  const char PROGMEM_CONFIG_APPLICATION_JSON[] PROGMEM = "application/json";
  const char PROGMEM_CONFIG_JSON_SUCCESS[] PROGMEM = "{\"success\":true}";
  const char PROGMEM_CONFIG_JSON_FAILURE_BEGINNING[] PROGMEM = "{\"success\":false,\"error\":\"";
  const char PROGMEM_CONFIG_JSON_FAILURE_END[] PROGMEM = "\"}";

}

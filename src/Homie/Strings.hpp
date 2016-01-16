#pragma once

// config mode

const char PROGMEM_CONFIG_CORS[] PROGMEM = "HTTP/1.1 204 No Content\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: PUT\r\nAccess-Control-Allow-Headers: Content-Type\r\n\r\n";
const char PROGMEM_CONFIG_APPLICATION_JSON[] PROGMEM = "application/json";
const char PROGMEM_CONFIG_JSON_FAILURE[] PROGMEM = "{\"success\":false}";

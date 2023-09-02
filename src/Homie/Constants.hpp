#pragma once

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif // ESP32

#ifndef HOMIE_CONFIG
#define HOMIE_CONFIG 1
#endif

// config mode requires SPIFFS as ESP Async Webserver only works with SPIFFS
#if HOMIE_CONFIG
#ifndef HOMIE_SPIFFS
#define HOMIE_SPIFFS
#endif
#endif

// default should be SPIFFS, except using LittleFS is explicitely defined
#ifndef HOMIE_LITTLEFS
#ifndef HOMIE_SPIFFS
#define HOMIE_SPIFFS
#endif
#endif

// fail if none or both are defined
#if defined(HOMIE_SPIFFS) && defined(HOMIE_LITTLEFS)
#error "Only one of HOMIE_SPIFFS and HOMIE_LITTLEFS must be defined. HOMIE_CONFIG requires HOMIE_SPIFFS."
#endif
#if !(defined(HOMIE_SPIFFS) || defined(HOMIE_LITTLEFS))
#error "At least one of HOMIE_SPIFFS or HOMIE_LITTLEFS needs to be defined."
#endif

namespace HomieInternals {
  const char HOMIE_VERSION[] = "3.0.1";
  const char HOMIE_ESP8266_VERSION[] = "3.0.0";

  const IPAddress ACCESS_POINT_IP(192, 168, 123, 1);

  const uint16_t DEFAULT_MQTT_PORT = 1883;
  const char DEFAULT_MQTT_BASE_TOPIC[] = "homie/";

  const uint8_t DEFAULT_RESET_PIN = 0;  // == D3 on nodeMCU
  const uint8_t DEFAULT_RESET_STATE = LOW;
  const uint16_t DEFAULT_RESET_TIME = 5 * 1000;

  const char DEFAULT_BRAND[] = "Homie";

  const uint16_t CONFIG_SCAN_INTERVAL = 20 * 1000;
  const uint32_t STATS_SEND_INTERVAL_SEC = 1 * 60;
  const uint16_t MQTT_RECONNECT_INITIAL_INTERVAL = 1000;
  const uint8_t MQTT_RECONNECT_MAX_BACKOFF = 6;

  const float LED_WIFI_DELAY = 1;
  const float LED_MQTT_DELAY = 0.2;

  const char CONFIG_UI_BUNDLE_PATH[] = "/homie/ui_bundle.gz";
  const char CONFIG_NEXT_BOOT_MODE_FILE_PATH[] = "/homie/NEXTMODE";
  const char CONFIG_FILE_PATH[] = "/homie/config.json";
}  // namespace HomieInternals

#pragma once

#include <ArduinoJson.h>

namespace HomieInternals {
  const char VERSION[] = "1.0.0";
  const int BAUD_RATE = 115200;

  const uint16_t DEFAULT_MQTT_PORT = 35589;
  const uint16_t DEFAULT_OTA_PORT = 35590;
  const char DEFAULT_OTA_PATH[] = "/ota";

  const uint8_t DEFAULT_RESET_PIN = 0; // == D3 on nodeMCU
  const byte DEFAULT_RESET_STATE = LOW;
  const uint16_t DEFAULT_RESET_TIME = 5 * 1000;

  const char DEFAULT_FW_NAME[] = "undefined";
  const char DEFAULT_FW_VERSION[] = "undefined";

  const unsigned int CONFIG_SCAN_INTERVAL = 20 * 1000;
  const unsigned int WIFI_RECONNECT_INTERVAL = 20 * 1000;
  const unsigned int MQTT_RECONNECT_INTERVAL = 5 * 1000;
  const unsigned long SIGNAL_QUALITY_SEND_INTERVAL = 300 * 1000;

  const float LED_WIFI_DELAY = 1;
  const float LED_MQTT_DELAY = 0.2;

  const unsigned int JSON_CONFIG_MAX_BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(6); // Max 4 elements at root, 2 elements in nested, etc...
  const char CONFIG_FILE_PATH[] = "/homie/config.json";
  const char CONFIG_OTA_PATH[] = "/homie/ota";
  const unsigned int CONFIG_MAX_LENGTH_NAME = 32 + 1;
  const unsigned int CONFIG_MAX_LENGTH_WIFI_SSID = 32 + 1;
  const unsigned int CONFIG_MAX_LENGTH_WIFI_PASSWORD = 63 + 1;
  const unsigned int CONFIG_MAX_LENGTH_MQTT_HOST = 63 + 1;
  const unsigned int CONFIG_MAX_LENGTH_MQTT_USERNAME = 63 + 1;
  const unsigned int CONFIG_MAX_LENGTH_MQTT_PASSWORD = 63 + 1;
  const unsigned int CONFIG_MAX_LENGTH_MQTT_FINGERPRINT = 59 + 1;
  const unsigned int CONFIG_MAX_LENGTH_OTA_HOST = CONFIG_MAX_LENGTH_MQTT_HOST;
  const unsigned int CONFIG_MAX_LENGTH_OTA_PATH = 63 + 1;
  const unsigned int CONFIG_MAX_LENGTH_OTA_FINGERPRINT = CONFIG_MAX_LENGTH_MQTT_FINGERPRINT;

  enum BootMode : byte {
    BOOT_NORMAL = 1,
    BOOT_CONFIG = 2,
    BOOT_OTA = 3
  };
}

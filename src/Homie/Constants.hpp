#pragma once

#include <ESP8266WiFi.h>

namespace HomieInternals {
  const char VERSION[] = "1.5.0";
  const unsigned long BAUD_RATE = 115200;

  const IPAddress ACCESS_POINT_IP(192, 168, 1, 1);

  const unsigned int DEFAULT_MQTT_PORT = 1883;
  const unsigned char DEFAULT_OTA_PORT = 80;
  const char DEFAULT_MQTT_BASE_TOPIC[] = "devices/";
  const char DEFAULT_OTA_PATH[] = "/ota";

  const unsigned char DEFAULT_RESET_PIN = 0; // == D3 on nodeMCU
  const unsigned char DEFAULT_RESET_STATE = LOW;
  const unsigned int DEFAULT_RESET_TIME = 5 * 1000;

  const char DEFAULT_BRAND[] = "Homie";
  const char DEFAULT_FW_NAME[] = "undefined";
  const char DEFAULT_FW_VERSION[] = "undefined";

  const unsigned int CONFIG_SCAN_INTERVAL = 20 * 1000;
  const unsigned int WIFI_RECONNECT_INTERVAL = 20 * 1000;
  const unsigned int MQTT_RECONNECT_INTERVAL = 5 * 1000;
  const unsigned long SIGNAL_QUALITY_SEND_INTERVAL = 5 * 60 * 1000;
  const unsigned long UPTIME_SEND_INTERVAL = 2 * 60 * 1000;

  const float LED_WIFI_DELAY = 1;
  const float LED_MQTT_DELAY = 0.2;

  const char CONFIG_FILE_PATH[] = "/homie/config.json";
  const char CONFIG_OTA_PATH[] = "/homie/ota";

  enum BootMode : unsigned char {
    BOOT_NORMAL = 1,
    BOOT_CONFIG,
    BOOT_OTA
  };
}

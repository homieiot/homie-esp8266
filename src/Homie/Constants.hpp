#pragma once

#include <ESP8266WiFi.h>

namespace HomieInternals {
  const char HOMIE_VERSION[] = "2.0.0";
  const char HOMIE_ESP8266_VERSION[] = "2.0.0";

  const IPAddress ACCESS_POINT_IP(192, 168, 1, 1);

  const uint16_t DEFAULT_MQTT_PORT = 1883;
  const char DEFAULT_MQTT_BASE_TOPIC[] = "homie/";

  const uint8_t DEFAULT_RESET_PIN = 0;  // == D3 on nodeMCU
  const uint8_t DEFAULT_RESET_STATE = LOW;
  const uint16_t DEFAULT_RESET_TIME = 5 * 1000;

  const char DEFAULT_BRAND[] = "Homie";

  const uint16_t CONFIG_SCAN_INTERVAL = 20 * 1000;
  const uint32_t SIGNAL_QUALITY_SEND_INTERVAL = 5 * 60 * 1000;
  const uint32_t UPTIME_SEND_INTERVAL = 2 * 60 * 1000;

  const float LED_WIFI_DELAY = 1;
  const float LED_MQTT_DELAY = 0.2;

  const char CONFIG_UI_BUNDLE_PATH[] = "/homie/ui_bundle.gz";
  const char CONFIG_BYPASS_STANDALONE_FILE_PATH[] = "/homie/BYPASS_STANDALONE";
  const char CONFIG_FILE_PATH[] = "/homie/config.json";

  enum class BootMode : uint8_t {
    STANDALONE = 1,
    NORMAL,
    CONFIG
  };
}  // namespace HomieInternals

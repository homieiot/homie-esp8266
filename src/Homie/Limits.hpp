#pragma once

#include <ArduinoJson.h>

namespace HomieInternals {
  const uint16_t MAX_JSON_CONFIG_FILE_SIZE = 500;

  // max setting elements
  const uint8_t MAX_CONFIG_SETTING_SIZE = 10;
  // 6 elements at root, 9 elements at wifi, 6 elements at mqtt, 1 element at ota, max settings elements
  const uint16_t MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE = JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(MAX_CONFIG_SETTING_SIZE);
  const uint16_t MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE = MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE + MAX_JSON_CONFIG_FILE_SIZE;

  const uint8_t MAX_IP_LENGTH = 4;
  const uint8_t MAX_IP_STRING_LENGTH = 16 + 1;

  const uint8_t MAX_MAC_LENGTH = 6;
  const uint8_t MAX_MAC_STRING_LENGTH = (MAX_MAC_LENGTH * 2) + 1;
  const uint8_t MAX_MAC_FORMATTED_STRING_LENGTH = (MAX_MAC_LENGTH * 2) + 5 + 1;

  const uint8_t MAX_WIFI_SSID_STRING_LENGTH = 32 + 1;
  const uint8_t MAX_WIFI_PASSWORD_STRING_LENGTH = 64 + 1;
  const uint16_t MAX_HOSTNAME_STRING_LENGTH = 255 + 1;
  const uint8_t MAX_FINGERPRINT_SIZE = 20;
  const uint8_t MAX_FINGERPRINT_STRING_LENGTH = (MAX_FINGERPRINT_SIZE *2) + 1;

  const uint8_t MAX_MQTT_CREDS_STRING_LENGTH = 32 + 1;
  const uint8_t MAX_MQTT_BASE_TOPIC_STRING_LENGTH = 48 + 1;
  const uint8_t MAX_MQTT_TOPIC_STRING_LENGTH = 128 + 1;

  const uint8_t MAX_FRIENDLY_NAME_STRING_LENGTH = 64 + 1;
  const uint8_t MAX_DEVICE_ID_STRING_LENGTH = 32 + 1;

  const uint8_t MAX_BRAND_STRING_LENGTH = (MAX_WIFI_SSID_STRING_LENGTH - 1) - ((MAX_MAC_STRING_LENGTH - 1) - 1) + 1;
  const uint8_t MAX_FIRMWARE_NAME_STRING_LENGTH = 32 + 1;
  const uint8_t MAX_FIRMWARE_VERSION_STRING_LENGTH = 16 + 1;

  const uint8_t MAX_NODE_ID_STRING_LENGTH = 24 + 1;
  const uint8_t MAX_NODE_TYPE_STRING_LENGTH = 24 + 1;
  const uint8_t MAX_NODE_PROPERTY_STRING_LENGTH = 24 + 1;

}  // namespace HomieInternals

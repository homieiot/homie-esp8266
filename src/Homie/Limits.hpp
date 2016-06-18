#pragma once

#include <ArduinoJson.h>

namespace HomieInternals {
  const uint16_t MAX_JSON_CONFIG_FILE_BUFFER_SIZE = 1000;
  const uint16_t MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(6); // Max 5 elements at root, 2 elements in nested, etc...

  const uint8_t MAX_WIFI_SSID_LENGTH = 32 + 1;
  const uint8_t MAX_WIFI_PASSWORD_LENGTH = 63 + 1;
  const uint8_t MAX_HOSTNAME_LENGTH = sizeof("super.long.domain-name.superhost.com");
  const uint8_t MAX_FINGERPRINT_LENGTH = 59 + 1;

  const uint8_t MAX_MQTT_CREDS_LENGTH = 32 + 1;
  const uint8_t MAX_MQTT_BASE_TOPIC_LENGTH = sizeof("shared-broker/username-lolipop/homie/sensors/");

  const uint8_t MAX_FRIENDLY_NAME_LENGTH = sizeof("My awesome friendly name of the living room");
  const uint8_t MAX_DEVICE_ID_LENGTH = sizeof("my-awesome-device-id-living-room");

  const uint8_t MAX_BRAND_LENGTH = MAX_WIFI_SSID_LENGTH - sizeof("-0123abcd") + 1;
  const uint8_t MAX_FIRMWARE_NAME_LENGTH = sizeof("my-awesome-home-firmware-name");
  const uint8_t MAX_FIRMWARE_VERSION_LENGTH = sizeof("v1.0.0-alpha+001");

  const uint8_t MAX_NODE_ID_LENGTH = sizeof("my-super-awesome-node-id");
  const uint8_t MAX_NODE_TYPE_LENGTH = sizeof("my-super-awesome-type");
  const uint8_t MAX_NODE_PROPERTY_LENGTH = sizeof("my-super-awesome-property");

  const uint8_t MAX_SUBSCRIPTIONS_COUNT_PER_NODE = 5;
}

#pragma once

#include "../Constants.hpp"

namespace HomieInternals {
  struct ConfigStruct {
    char name[CONFIG_MAX_LENGTH_NAME];
    struct WiFi {
      char ssid[CONFIG_MAX_LENGTH_WIFI_SSID];
      char password[CONFIG_MAX_LENGTH_WIFI_PASSWORD];
    } wifi;
    struct MQTT {
      char host[CONFIG_MAX_LENGTH_MQTT_HOST];
      uint16_t port;
      bool auth;
      char username[CONFIG_MAX_LENGTH_MQTT_USERNAME];
      char password[CONFIG_MAX_LENGTH_MQTT_PASSWORD];
      bool ssl;
      char fingerprint[CONFIG_MAX_LENGTH_MQTT_FINGERPRINT];
    } mqtt;
    struct OTA {
      bool enabled;
      char host[CONFIG_MAX_LENGTH_OTA_HOST];
      uint16_t port;
      char path[CONFIG_MAX_LENGTH_OTA_PATH];
      bool ssl;
      char fingerprint[CONFIG_MAX_LENGTH_OTA_FINGERPRINT];
    } ota;
  };
}

#pragma once

#include "../Constants.hpp"

namespace HomieInternals {
  struct ConfigStruct {
    bool configured;
    BootMode boot_mode;
    char name[EEPROM_LENGTH_NAME];
    struct WiFi {
      char ssid[EEPROM_LENGTH_WIFI_SSID];
      char password[EEPROM_LENGTH_WIFI_PASSWORD];
    } wifi;
    struct MQTT {
      char host[EEPROM_LENGTH_MQTT_HOST];
      uint16_t port;
      bool auth;
      char username[EEPROM_LENGTH_MQTT_USERNAME];
      char password[EEPROM_LENGTH_MQTT_PASSWORD];
      bool ssl;
      char fingerprint[EEPROM_LENGTH_MQTT_FINGERPRINT];
    } mqtt;
    struct OTA {
      bool enabled;
      char host[EEPROM_LENGTH_OTA_HOST];
      uint16_t port;
      char path[EEPROM_LENGTH_OTA_PATH];
      bool ssl;
      char fingerprint[EEPROM_LENGTH_OTA_FINGERPRINT];
    } ota;
  };

  const int EEPROM_CONFIG_SIZE = sizeof(ConfigStruct);
}

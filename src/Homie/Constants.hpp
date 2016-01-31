#pragma once

#include <vector>
#include "../3rd/PubSubClient/src/PubSubClient.h"

namespace HomieInternals {
  const int BAUD_RATE = 115200;

  const uint16_t DEFAULT_MQTT_PORT = 35589;
  const uint16_t DEFAULT_OTA_PORT = 35590;
  const char DEFAULT_OTA_PATH[] = "/ota";

  const float PIN_RESET = D3;

  const float LED_WIFI_DELAY = 1;
  const float LED_MQTT_DELAY = 0.2;

  const int EEPROM_OFFSET = 0;
  const int EEPROM_LENGTH_NAME = 32 + 1;
  const int EEPROM_LENGTH_WIFI_SSID = 32 + 1;
  const int EEPROM_LENGTH_WIFI_PASSWORD = 63 + 1;
  const int EEPROM_LENGTH_MQTT_HOST = 63 + 1;
  const int EEPROM_LENGTH_MQTT_USERNAME = 63 + 1;
  const int EEPROM_LENGTH_MQTT_PASSWORD = 63 + 1;
  const int EEPROM_LENGTH_MQTT_FINGERPRINT = 59 + 1;
  const int EEPROM_LENGTH_OTA_HOST = EEPROM_LENGTH_MQTT_HOST;
  const int EEPROM_LENGTH_OTA_PATH = 63 + 1;
  const int EEPROM_LENGTH_OTA_FINGERPRINT = EEPROM_LENGTH_MQTT_FINGERPRINT;

  enum BootMode : byte {
    BOOT_NORMAL = 1,
    BOOT_CONFIG = 2,
    BOOT_OTA = 3
  };

  typedef struct {
    char* name;
    char* type;
  } Node;

  typedef struct {
    char* node;
    char* property;
  } Subscription;

  struct SharedInterface {
    int eepromCount;
    char* fwname;
    char* fwversion;
    bool resettable;
    bool readyToOperate;
    std::vector<Node> nodes;
    std::vector<Subscription> subscriptions;
    void (*inputHandler)(String node, String property, String message);
    void (*setupFunction)(void);
    void (*loopFunction)(void);
    void (*resetFunction)(void);
    PubSubClient* mqtt;
  };

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

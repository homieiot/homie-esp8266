#pragma once

#include <vector>
#include "../3rd/PubSubClient/src/PubSubClient.h"

const int BAUD_RATE = 115200;

const float PIN_RESET = D3;

const float LED_WIFI_DELAY = 1;
const float LED_MQTT_DELAY = 0.2;

const int EEPROM_OFFSET = 0;
const int EEPROM_LENGTH_HOSTNAME = 32 + 1;
const int EEPROM_LENGTH_WIFI_SSID = 32 + 1;
const int EEPROM_LENGTH_WIFI_PASSWORD = 63 + 1;
const int EEPROM_LENGTH_HOMIE_HOST = 63 + 1;

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
  char* version;
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
  char hostname[EEPROM_LENGTH_HOSTNAME];
  char wifi_ssid[EEPROM_LENGTH_WIFI_SSID];
  char wifi_password[EEPROM_LENGTH_WIFI_PASSWORD];
  char homie_host[EEPROM_LENGTH_HOMIE_HOST];
};

const int EEPROM_CONFIG_SIZE = sizeof(ConfigStruct);

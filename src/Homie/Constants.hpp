#pragma once

namespace HomieInternals {
  const int BAUD_RATE = 115200;

  const uint16_t DEFAULT_MQTT_PORT = 35589;
  const uint16_t DEFAULT_OTA_PORT = 35590;
  const char DEFAULT_OTA_PATH[] = "/ota";

  const uint8_t PIN_RESET = 0; // == D3 on nodeMCU

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
}

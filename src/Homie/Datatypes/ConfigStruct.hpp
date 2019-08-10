#pragma once

#include "../Constants.hpp"
#include "../Limits.hpp"

namespace HomieInternals {
struct ConfigStruct {
  char name[MAX_FRIENDLY_NAME_LENGTH];
  char deviceId[MAX_DEVICE_ID_LENGTH];
  uint16_t deviceStatsInterval;

  struct WiFi {
    char ssid[MAX_WIFI_SSID_LENGTH];
    char password[MAX_WIFI_PASSWORD_LENGTH];
    char bssid[MAX_MAC_STRING_LENGTH + 6];
    uint16_t channel;
    char ip[MAX_IP_STRING_LENGTH];
    char mask[MAX_IP_STRING_LENGTH];
    char gw[MAX_IP_STRING_LENGTH];
    char dns1[MAX_IP_STRING_LENGTH];
    char dns2[MAX_IP_STRING_LENGTH];
  } wifi;

  struct MQTT {
    struct Server {
      char host[MAX_HOSTNAME_LENGTH];
      uint16_t port;
#if ASYNC_TCP_SSL_ENABLED
      struct {
        bool enabled;
        bool hasFingerprint;
        uint8_t fingerprint[MAX_FINGERPRINT_SIZE];
      } ssl;
#endif
    } server;
    char baseTopic[MAX_MQTT_BASE_TOPIC_LENGTH];
    bool auth;
    char username[MAX_MQTT_CREDS_LENGTH];
    char password[MAX_MQTT_CREDS_LENGTH];
  } mqtt;

  struct OTA {
    bool enabled;
  } ota;
};
}  // namespace HomieInternals

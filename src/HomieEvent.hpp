#pragma once

#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>

enum class HomieEventType : uint8_t {
  STANDALONE_MODE = 1,
  CONFIGURATION_MODE,
  NORMAL_MODE,
  OTA_STARTED,
  OTA_PROGRESS,
  OTA_SUCCESSFUL,
  OTA_FAILED,
  ABOUT_TO_RESET,
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
  MQTT_READY,
  MQTT_DISCONNECTED,
  MQTT_PACKET_ACKNOWLEDGED,
  READY_TO_SLEEP,
  SENDING_STATISTICS
};

struct HomieEvent {
  HomieEventType type;
  /* WIFI_CONNECTED */
  IPAddress ip;
  IPAddress mask;
  IPAddress gateway;
  /* WIFI_DISCONNECTED */
  WiFiDisconnectReason wifiReason;
  /* MQTT_DISCONNECTED */
  AsyncMqttClientDisconnectReason mqttReason;
  /* MQTT_PACKET_ACKNOWLEDGED */
  uint16_t packetId;
  /* OTA_PROGRESS */
  size_t sizeDone;
  size_t sizeTotal;
};

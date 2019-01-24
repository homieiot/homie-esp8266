#ifndef HOMIE_MDNS
#define HOMIE_MDNS 1
#endif

#pragma once

#include "Arduino.h"

#include <functional>
#include <libb64/cdecode.h>
#include <ESP8266WiFi.h>
#if HOMIE_MDNS
#include <ESP8266mDNS.h>
#endif
#include <AsyncMqttClient.h>
#include "../../HomieNode.hpp"
#include "../../HomieRange.hpp"
#include "../../StreamingOperator.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Utils/Helpers.hpp"
#include "../Uptime.hpp"
#include "../Timer.hpp"
#include "../ExponentialBackoffTimer.hpp"
#include "Boot.hpp"
#include "../Utils/ResetHandler.hpp"

namespace HomieInternals {
class BootNormal : public Boot {
 public:
  BootNormal();
  ~BootNormal();
  void setup();
  void loop();

 private:
  struct AdvertisementProgress {
    bool done = false;
    enum class GlobalStep {
      PUB_HOMIE,
      PUB_NAME,
      PUB_MAC,
      PUB_LOCALIP,
      PUB_NODES_ATTR,
      PUB_STATS_INTERVAL,
      PUB_FW_NAME,
      PUB_FW_VERSION,
      PUB_FW_CHECKSUM,
      PUB_IMPLEMENTATION,
      PUB_IMPLEMENTATION_CONFIG,
      PUB_IMPLEMENTATION_VERSION,
      PUB_IMPLEMENTATION_OTA_ENABLED,
      PUB_NODES,
      SUB_IMPLEMENTATION_OTA,
      SUB_IMPLEMENTATION_RESET,
      SUB_IMPLEMENTATION_CONFIG_SET,
      SUB_SET,
      SUB_BROADCAST,
      PUB_ONLINE
    } globalStep;

    enum class NodeStep {
      PUB_TYPE,
      PUB_PROPERTIES
    } nodeStep;

    size_t currentNodeIndex;
  } _advertisementProgress;
  Uptime _uptime;
  Timer _statsTimer;
  ExponentialBackoffTimer _mqttReconnectTimer;
  bool _setupFunctionCalled;
  WiFiEventHandler _wifiGotIpHandler;
  WiFiEventHandler _wifiDisconnectedHandler;
  bool _mqttConnectNotified;
  bool _mqttDisconnectNotified;
  bool _otaOngoing;
  bool _flaggedForReboot;
  uint16_t _mqttOfflineMessageId;
  char _fwChecksum[32 + 1];
  bool _otaIsBase64;
  base64_decodestate _otaBase64State;
  size_t _otaBase64Pads;
  size_t _otaSizeTotal;
  size_t _otaSizeDone;

  std::unique_ptr<char[]> _mqttTopic;

  std::unique_ptr<char[]> _mqttClientId;
  std::unique_ptr<char[]> _mqttWillTopic;
  std::unique_ptr<char[]> _mqttPayloadBuffer;
  std::unique_ptr<char*[]> _mqttTopicLevels;
  uint8_t _mqttTopicLevelsCount;

  void _wifiConnect();
  void _onWifiGotIp(const WiFiEventStationModeGotIP& event);
  void _onWifiDisconnected(const WiFiEventStationModeDisconnected& event);
  void _mqttConnect();
  void _advertise();
  void _onMqttConnected();
  void _onMqttDisconnected(AsyncMqttClientDisconnectReason reason);
  void _onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  void _onMqttPublish(uint16_t id);
  void _prefixMqttTopic();
  char* _prefixMqttTopic(PGM_P topic);
  bool _publishOtaStatus(int status, const char* info = nullptr);
  void _endOtaUpdate(bool success, uint8_t update_error = UPDATE_ERROR_OK);

  // _onMqttMessage Helpers
  void __splitTopic(char* topic);
  bool __fillPayloadBuffer(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleOTAUpdates(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleBroadcasts(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleResets(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleConfig(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
  bool __handleNodeProperty(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total);
};
}  // namespace HomieInternals

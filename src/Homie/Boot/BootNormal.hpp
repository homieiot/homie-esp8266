#pragma once

#include "Arduino.h"

#include <functional>
#include <libb64/cdecode.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <AsyncMqttClient.h>
#include <Bounce2.h>
#include "../../HomieNode.hpp"
#include "../../HomieRange.hpp"
#include "../../StreamingOperator.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Utils/Helpers.hpp"
#include "../Uptime.hpp"
#include "../Timer.hpp"
#include "../TimedRetry.hpp"
#include "Boot.hpp"

namespace HomieInternals {
class BootNormal : public Boot {
 public:
  BootNormal();
  ~BootNormal();
  void setup();
  void loop();

 private:
  Uptime _uptime;
  Timer _statsTimer;
  TimedRetry _mqttTimedRetry;
  bool _setupFunctionCalled;
  WiFiEventHandler _wifiGotIpHandler;
  WiFiEventHandler _wifiDisconnectedHandler;
  bool _mqttDisconnectNotified;
  bool _flaggedForOta;
  bool _flaggedForReset;
  bool _flaggedForReboot;
  Bounce _resetDebouncer;
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

  void _handleReset();
  void _wifiConnect();
  void _onWifiGotIp(const WiFiEventStationModeGotIP& event);
  void _onWifiDisconnected(const WiFiEventStationModeDisconnected& event);
  void _mqttConnect();
  void _onMqttConnected();
  void _onMqttDisconnected(AsyncMqttClientDisconnectReason reason);
  void _onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  void _onMqttPublish(uint16_t id);
  void _prefixMqttTopic();
  char* _prefixMqttTopic(PGM_P topic);
  uint16_t _publishOtaStatus(int status, const char* info = nullptr);
  uint16_t _publishOtaStatus_P(int status, PGM_P info);
  void _endOtaUpdate(bool success, uint8_t update_error = UPDATE_ERROR_OK);
  void _stringToBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);
};
}  // namespace HomieInternals

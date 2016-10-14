#pragma once

#include "Arduino.h"

#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <AsyncMqttClient.h>
#include <Bounce2.h>
#include "../../HomieNode.hpp"
#include "../../HomieRange.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Datatypes/Interface.hpp"
#include "../Helpers.hpp"
#include "../Config.hpp"
#include "../Blinker.hpp"
#include "../Uptime.hpp"
#include "../Timer.hpp"
#include "../Logger.hpp"
#include "Boot.hpp"

namespace HomieInternals {
class BootNormal : public Boot {
 public:
  BootNormal();
  ~BootNormal();
  void setup();
  void loop();
  void prepareToSleep();

 private:
  Uptime _uptime;
  Timer _signalQualityTimer;
  Timer _uptimeTimer;
  bool _setupFunctionCalled;
  WiFiEventHandler _wifiGotIpHandler;
  WiFiEventHandler _wifiDisconnectedHandler;
  bool _mqttDisconnectNotified;
  bool _flaggedForOta;
  bool _flaggedForReset;
  bool _flaggedForReboot;
  Bounce _resetDebouncer;
  bool _flaggedForSleep;
  uint16_t _mqttOfflineMessageId;

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
  uint16_t _publishOtaStatus(int status, const char* info=0);
  uint16_t _publishOtaStatus_P(int status, PGM_P info);
};
}  // namespace HomieInternals

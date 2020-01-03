#include "BootNormal.hpp"

using namespace HomieInternals;

BootNormal::BootNormal()
  : Boot("normal")
  , _mqttReconnectTimer(MQTT_RECONNECT_INITIAL_INTERVAL, MQTT_RECONNECT_MAX_BACKOFF)
  , _setupFunctionCalled(false)
  , _mqttConnectNotified(false)
  , _mqttDisconnectNotified(true)
  , _otaOngoing(false)
  , _flaggedForReboot(false)
  , _mqttOfflineMessageId(0)
  , _otaIsBase64(false)
  , _otaBase64Pads(0)
  , _otaSizeTotal(0)
  , _otaSizeDone(0)
  , _mqttTopic(nullptr)
  , _mqttClientId(nullptr)
  , _mqttWillTopic(nullptr)
  , _mqttPayloadBuffer(nullptr)
  , _mqttTopicLevels(nullptr)
  , _mqttTopicLevelsCount(0)
  , _mqttTopicCopy(nullptr) {
}

BootNormal::~BootNormal() {
}

void BootNormal::setup() {
  Boot::setup();

  strlcpy(_fwChecksum, ESP.getSketchMD5().c_str(), sizeof(_fwChecksum));
  _fwChecksum[sizeof(_fwChecksum) - 1] = '\0';

  #ifdef ESP32
  //FIXME
  #elif defined(ESP8266)
  Update.runAsync(true);
  #endif // ESP32

  _statsTimer.setInterval(Interface::get().getConfig().get().deviceStatsInterval * 1000);

  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);

  // Generate topic buffer
  size_t baseTopicLength = strlen(Interface::get().getConfig().get().mqtt.baseTopic) + strlen(Interface::get().getConfig().get().deviceId);
  size_t longestSubtopicLength = 31 + 1;  // /$implementation/ota/firmware/+
  for (HomieNode* iNode : HomieNode::nodes) {
    size_t nodeMaxTopicLength = 1 + strlen(iNode->getId()) + 12 + 1;  // /id/$properties
    if (nodeMaxTopicLength > longestSubtopicLength) longestSubtopicLength = nodeMaxTopicLength;

    for (Property* iProperty : iNode->getProperties()) {
      size_t propertyMaxTopicLength = 1 + strlen(iNode->getId()) + 1 + strlen(iProperty->getId()) + 1;
      if (iProperty->isSettable()) propertyMaxTopicLength += 4;  // /set

      if (propertyMaxTopicLength > longestSubtopicLength) longestSubtopicLength = propertyMaxTopicLength;
    }
  }
  _mqttTopic = std::unique_ptr<char[]>(new char[baseTopicLength + longestSubtopicLength]);

  #ifdef ESP32
  _wifiGotIpHandler = WiFi.onEvent(std::bind(&BootNormal::_onWifiGotIp, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  _wifiDisconnectedHandler = WiFi.onEvent(std::bind(&BootNormal::_onWifiDisconnected, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
  #elif defined(ESP8266)
  _wifiGotIpHandler = WiFi.onStationModeGotIP(std::bind(&BootNormal::_onWifiGotIp, this, std::placeholders::_1));
  _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&BootNormal::_onWifiDisconnected, this, std::placeholders::_1));
  #endif // ESP32

  Interface::get().getMqttClient().onConnect(std::bind(&BootNormal::_onMqttConnected, this));
  Interface::get().getMqttClient().onDisconnect(std::bind(&BootNormal::_onMqttDisconnected, this, std::placeholders::_1));
  Interface::get().getMqttClient().onMessage(std::bind(&BootNormal::_onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  Interface::get().getMqttClient().onPublish(std::bind(&BootNormal::_onMqttPublish, this, std::placeholders::_1));

  Interface::get().getMqttClient().setServer(Interface::get().getConfig().get().mqtt.server.host, Interface::get().getConfig().get().mqtt.server.port);

#if ASYNC_TCP_SSL_ENABLED
  Interface::get().getLogger() << "SSL is: " << Interface::get().getConfig().get().mqtt.server.ssl.enabled << endl;
  Interface::get().getMqttClient().setSecure(Interface::get().getConfig().get().mqtt.server.ssl.enabled);
  if (Interface::get().getConfig().get().mqtt.server.ssl.enabled && Interface::get().getConfig().get().mqtt.server.ssl.hasFingerprint) {
    char hexBuf[MAX_FINGERPRINT_STRING_LENGTH];
    Helpers::byteArrayToHexString(Interface::get().getConfig().get().mqtt.server.ssl.fingerprint, hexBuf, MAX_FINGERPRINT_SIZE);
    Interface::get().getLogger() << "Using fingerprint: " << hexBuf << endl;
    Interface::get().getMqttClient().addServerFingerprint((const uint8_t*)Interface::get().getConfig().get().mqtt.server.ssl.fingerprint);
  }
#endif

  Interface::get().getMqttClient().setMaxTopicLength(MAX_MQTT_TOPIC_LENGTH);
  _mqttClientId = std::unique_ptr<char[]>(new char[strlen(Interface::get().brand) + 1 + strlen(Interface::get().getConfig().get().deviceId) + 1]);
  strcpy(_mqttClientId.get(), Interface::get().brand);
  strcat_P(_mqttClientId.get(), PSTR("-"));
  strcat(_mqttClientId.get(), Interface::get().getConfig().get().deviceId);
  Interface::get().getMqttClient().setClientId(_mqttClientId.get());
  char* mqttWillTopic = _prefixMqttTopic(PSTR("/$state"));
  _mqttWillTopic = std::unique_ptr<char[]>(new char[strlen(mqttWillTopic) + 1]);
  memcpy(_mqttWillTopic.get(), mqttWillTopic, strlen(mqttWillTopic) + 1);
  Interface::get().getMqttClient().setWill(_mqttWillTopic.get(), 1, true, "lost");

  if (Interface::get().getConfig().get().mqtt.auth) Interface::get().getMqttClient().setCredentials(Interface::get().getConfig().get().mqtt.username, Interface::get().getConfig().get().mqtt.password);

#if HOMIE_CONFIG
  ResetHandler::Attach();
#endif

  Interface::get().getConfig().log();

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->setup();
  }

  _wifiConnect();
}

void BootNormal::loop() {
  Boot::loop();

  if (_flaggedForReboot && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;

    Interface::get().getLogger() << F("↻ Rebooting...") << endl;
    Serial.flush();
    ESP.restart();
  }

  for (HomieNode* iNode : HomieNode::nodes) {
    if (iNode->runLoopDisconnected ||Interface::get().getMqttClient().connected()) iNode->loop();
  }
  if (_mqttReconnectTimer.check()) {
    _mqttConnect();
    return;
  }

  if (!Interface::get().getMqttClient().connected()) return;

  // here, we are connected to the broker

  if (!_advertisementProgress.done) {
    _advertise();
    return;
  }

  // here, we finished the advertisement

  if (!_mqttConnectNotified) {
    Interface::get().ready = true;
    if (Interface::get().led.enabled) Interface::get().getBlinker().stop();

    Interface::get().getLogger() << F("✔ MQTT ready") << endl;
    Interface::get().getLogger() << F("Triggering MQTT_READY event...") << endl;
    Interface::get().event.type = HomieEventType::MQTT_READY;
    Interface::get().eventHandler(Interface::get().event);

    for (HomieNode* iNode : HomieNode::nodes) {
      iNode->onReadyToOperate();
    }

    if (!_setupFunctionCalled) {
      Interface::get().getLogger() << F("Calling setup function...") << endl;
      Interface::get().setupFunction();
      _setupFunctionCalled = true;
    }

    _mqttConnectNotified = true;
    return;
  }

  // here, we have notified the sketch we are ready

  if (_mqttOfflineMessageId == 0 && Interface::get().flaggedForSleep) {
    Interface::get().getLogger() << F("Device in preparation to sleep...") << endl;
    _mqttOfflineMessageId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$state")), 1, true, "sleeping");
  }

  if (_statsTimer.check()) {
    char statsIntervalStr[3 + 1];
    itoa(Interface::get().getConfig().get().deviceStatsInterval+5, statsIntervalStr, 10);
    Interface::get().getLogger() << F("〽 Sending statistics...") << endl;
    Interface::get().getLogger() << F("  • Interval: ") << statsIntervalStr << F("s (") << Interface::get().getConfig().get().deviceStatsInterval << F("s including 5s grace time)") << endl;
    uint16_t intervalPacketId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats/interval")), 1, true, statsIntervalStr);

    uint8_t quality = Helpers::rssiToPercentage(WiFi.RSSI());
    char qualityStr[3 + 1];
    itoa(quality, qualityStr, 10);
    Interface::get().getLogger() << F("  • Wi-Fi signal quality: ") << qualityStr << F("%") << endl;
    uint16_t signalPacketId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats/signal")), 1, true, qualityStr);

    _uptime.update();
    char uptimeStr[20 + 1];
    itoa(_uptime.getSeconds(), uptimeStr, 10);
    Interface::get().getLogger() << F("  • Uptime: ") << uptimeStr << F("s") << endl;
    uint16_t uptimePacketId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats/uptime")), 1, true, uptimeStr);

    if (intervalPacketId != 0 && signalPacketId != 0 && uptimePacketId != 0) _statsTimer.tick();
    Interface::get().event.type = HomieEventType::SENDING_STATISTICS;
    Interface::get().eventHandler(Interface::get().event);
  }

  Interface::get().loopFunction();
}

void BootNormal::_prefixMqttTopic() {
  strcpy(_mqttTopic.get(), Interface::get().getConfig().get().mqtt.baseTopic);
  strcat(_mqttTopic.get(), Interface::get().getConfig().get().deviceId);
}

char* BootNormal::_prefixMqttTopic(PGM_P topic) {
  _prefixMqttTopic();
  strcat_P(_mqttTopic.get(), topic);

  return _mqttTopic.get();
}

bool BootNormal::_publishOtaStatus(int status, const char* info) {
  String payload(status);
  if (info) {
    payload.concat(F(" "));
    payload.concat(info);
  }

  return Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/ota/status")), 0, true, payload.c_str()) != 0;
}

void BootNormal::_endOtaUpdate(bool success, uint8_t update_error) {
  if (success) {
    Interface::get().getLogger() << F("✔ OTA succeeded") << endl;
    Interface::get().getLogger() << F("Triggering OTA_SUCCESSFUL event...") << endl;
    Interface::get().event.type = HomieEventType::OTA_SUCCESSFUL;
    Interface::get().eventHandler(Interface::get().event);

    _publishOtaStatus(200);  // 200 OK
    _flaggedForReboot = true;
  } else {
    int code;
    String info;
    switch (update_error) {
      case UPDATE_ERROR_SIZE:               // new firmware size is zero
      case UPDATE_ERROR_MAGIC_BYTE:         // new firmware does not have 0xE9 in first byte
      #ifdef ESP32
      //FIXME
      #elif defined(ESP8266)
      case UPDATE_ERROR_NEW_FLASH_CONFIG:   // bad new flash config (does not match flash ID)
        code = 400;  // 400 Bad Request
        info.concat(F("BAD_FIRMWARE"));
        break;
      #endif //ESP32
      case UPDATE_ERROR_MD5:
        code = 400;  // 400 Bad Request
        info.concat(F("BAD_CHECKSUM"));
        break;
      case UPDATE_ERROR_SPACE:
        code = 400;  // 400 Bad Request
        info.concat(F("NOT_ENOUGH_SPACE"));
        break;
      case UPDATE_ERROR_WRITE:
      case UPDATE_ERROR_ERASE:
      case UPDATE_ERROR_READ:
        code = 500;  // 500 Internal Server Error
        info.concat(F("FLASH_ERROR"));
        break;
      default:
        code = 500;  // 500 Internal Server Error
        info.concat(F("INTERNAL_ERROR "));
        info.concat(update_error);
        break;
    }
    _publishOtaStatus(code, info.c_str());

    Interface::get().getLogger() << F("✖ OTA failed (") << code << F(" ") << info << F(")") << endl;

    Interface::get().getLogger() << F("Triggering OTA_FAILED event...") << endl;
    Interface::get().event.type = HomieEventType::OTA_FAILED;
    Interface::get().eventHandler(Interface::get().event);
  }
  _otaOngoing = false;
}

void BootNormal::_wifiConnect() {
  if (!Interface::get().disable) {
    if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
    Interface::get().getLogger() << F("↕ Attempting to connect to Wi-Fi...") << endl;

    if (WiFi.getMode() != WIFI_STA) WiFi.mode(WIFI_STA);

    #ifdef ESP32
    WiFi.setHostname(Interface::get().getConfig().get().deviceId);
    #elif defined(ESP8266)
    WiFi.hostname(Interface::get().getConfig().get().deviceId);
    #endif // ESP32
    if (strcmp_P(Interface::get().getConfig().get().wifi.ip, PSTR("")) != 0) {  // on _validateConfigWifi there is a requirement for mask and gateway
      IPAddress convertedIp;
      convertedIp.fromString(Interface::get().getConfig().get().wifi.ip);
      IPAddress convertedMask;
      convertedMask.fromString(Interface::get().getConfig().get().wifi.mask);
      IPAddress convertedGateway;
      convertedGateway.fromString(Interface::get().getConfig().get().wifi.gw);

      if (strcmp_P(Interface::get().getConfig().get().wifi.dns1, PSTR("")) != 0) {
        IPAddress convertedDns1;
        convertedDns1.fromString(Interface::get().getConfig().get().wifi.dns1);
        if ((strcmp_P(Interface::get().getConfig().get().wifi.dns2, PSTR("")) != 0)) {  // on _validateConfigWifi there is requirement that we need dns1 if we want to define dns2
          IPAddress convertedDns2;
          convertedDns2.fromString(Interface::get().getConfig().get().wifi.dns2);
          WiFi.config(convertedIp, convertedGateway, convertedMask, convertedDns1, convertedDns2);
        } else {
          WiFi.config(convertedIp, convertedGateway, convertedMask, convertedDns1);
        }
      } else {
        WiFi.config(convertedIp, convertedGateway, convertedMask);
      }
    }

    if (strcmp_P(Interface::get().getConfig().get().wifi.bssid, PSTR("")) != 0) {
      byte bssidBytes[6];
      Helpers::stringToBytes(Interface::get().getConfig().get().wifi.bssid, ':', bssidBytes, 6, 16);
      WiFi.begin(Interface::get().getConfig().get().wifi.ssid, Interface::get().getConfig().get().wifi.password, Interface::get().getConfig().get().wifi.channel, bssidBytes);
    } else {
      WiFi.begin(Interface::get().getConfig().get().wifi.ssid, Interface::get().getConfig().get().wifi.password);
    }

    #ifdef ESP32
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    #elif defined(ESP8266)
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    #endif // ESP32
  }
}

#ifdef ESP32
void BootNormal::_onWifiGotIp(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
  Interface::get().getLogger() << F("✔ Wi-Fi connected, IP: ") << IPAddress(info.got_ip.ip_info.ip.addr) << endl;
  Interface::get().getLogger() << F("Triggering WIFI_CONNECTED event...") << endl;
  Interface::get().event.type = HomieEventType::WIFI_CONNECTED;
  Interface::get().event.ip = IPAddress(info.got_ip.ip_info.ip.addr);
  Interface::get().event.mask = IPAddress(info.got_ip.ip_info.netmask.addr);
  Interface::get().event.gateway = IPAddress(info.got_ip.ip_info.gw.addr);
  Interface::get().eventHandler(Interface::get().event);
  MDNS.begin(Interface::get().getConfig().get().deviceId);

  _mqttConnect();
}
#elif defined(ESP8266)
void BootNormal::_onWifiGotIp(const WiFiEventStationModeGotIP& event) {
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
  Interface::get().getLogger() << F("✔ Wi-Fi connected, IP: ") << event.ip << endl;
  Interface::get().getLogger() << F("Triggering WIFI_CONNECTED event...") << endl;
  Interface::get().event.type = HomieEventType::WIFI_CONNECTED;
  Interface::get().event.ip = event.ip;
  Interface::get().event.mask = event.mask;
  Interface::get().event.gateway = event.gw;
  Interface::get().eventHandler(Interface::get().event);
#if HOMIE_MDNS
  MDNS.begin(Interface::get().getConfig().get().deviceId);
#endif

  _mqttConnect();
}
#endif // ESP32

#ifdef ESP32
void BootNormal::_onWifiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Interface::get().ready = false;
  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
  _statsTimer.deactivate();
  Interface::get().getLogger() << F("✖ Wi-Fi disconnected, reason: ") << info.disconnected.reason << endl;
  Interface::get().getLogger() << F("Triggering WIFI_DISCONNECTED event...") << endl;
  Interface::get().event.type = HomieEventType::WIFI_DISCONNECTED;
  Interface::get().event.wifiReason = info.disconnected.reason;
  Interface::get().eventHandler(Interface::get().event);

  _wifiConnect();
}
#elif defined(ESP8266)
void BootNormal::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event) {
  Interface::get().ready = false;
  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
  _statsTimer.deactivate();
  Interface::get().getLogger() << F("✖ Wi-Fi disconnected, reason: ") << event.reason << endl;
  Interface::get().getLogger() << F("Triggering WIFI_DISCONNECTED event...") << endl;
  Interface::get().event.type = HomieEventType::WIFI_DISCONNECTED;
  Interface::get().event.wifiReason = event.reason;
  Interface::get().eventHandler(Interface::get().event);

  _wifiConnect();
}
#endif // ESP32

void BootNormal::_mqttConnect() {
  if (!Interface::get().disable) {
    if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_MQTT_DELAY);
    _mqttConnectNotified = false;
    Interface::get().getLogger() << F("↕ Attempting to connect to MQTT...") << endl;
    Interface::get().getMqttClient().connect();
  }
}

void BootNormal::_advertise() {
  uint16_t packetId;
  switch (_advertisementProgress.globalStep) {
    case AdvertisementProgress::GlobalStep::PUB_INIT:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$state")), 1, true, "init");
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_HOMIE;
      break;
    case AdvertisementProgress::GlobalStep::PUB_HOMIE:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$homie")), 1, true, HOMIE_VERSION);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_NAME;
      break;
    case AdvertisementProgress::GlobalStep::PUB_NAME:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$name")), 1, true, Interface::get().getConfig().get().name);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_MAC;
      break;
    case AdvertisementProgress::GlobalStep::PUB_MAC:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$mac")), 1, true, WiFi.macAddress().c_str());
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_LOCALIP;
      break;
    case AdvertisementProgress::GlobalStep::PUB_LOCALIP:
    {
      IPAddress localIp = WiFi.localIP();
      char localIpStr[MAX_IP_STRING_LENGTH];
      Helpers::ipToString(localIp, localIpStr);
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$localip")), 1, true, localIpStr);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_NODES_ATTR;
      break;
    }
    case AdvertisementProgress::GlobalStep::PUB_NODES_ATTR:
    {
      String nodes;
      for (HomieNode* node : HomieNode::nodes) {
        nodes.concat(node->getId());
        if (node->isRange())
          nodes.concat(F("[]"));
        nodes.concat(F(","));
      }
      if (HomieNode::nodes.size() >= 1) nodes.remove(nodes.length() - 1);
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$nodes")), 1, true, nodes.c_str());
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_STATS;
      break;
    }
    case AdvertisementProgress::GlobalStep::PUB_STATS:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats")), 1, true, "uptime");
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_STATS_INTERVAL;
      break;
    case AdvertisementProgress::GlobalStep::PUB_STATS_INTERVAL:
      char statsIntervalStr[3 + 1];
      itoa(Interface::get().getConfig().get().deviceStatsInterval+5, statsIntervalStr, 10);
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats/interval")), 1, true, statsIntervalStr);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_FW_NAME;
      break;
    case AdvertisementProgress::GlobalStep::PUB_FW_NAME:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$fw/name")), 1, true, Interface::get().firmware.name);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_FW_VERSION;
      break;
    case AdvertisementProgress::GlobalStep::PUB_FW_VERSION:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$fw/version")), 1, true, Interface::get().firmware.version);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_FW_CHECKSUM;
      break;
    case AdvertisementProgress::GlobalStep::PUB_FW_CHECKSUM:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$fw/checksum")), 1, true, _fwChecksum);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION;
      break;
    case AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION:
      #ifdef ESP32
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation")), 1, true, "esp32");
      #elif defined(ESP8266)
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation")), 1, true, "esp8266");
      #endif // ESP32
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION_CONFIG;
      break;
    case AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION_CONFIG:
    {
      char* safeConfigFile = Interface::get().getConfig().getSafeConfigFile();
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/config")), 1, true, safeConfigFile);
      delete safeConfigFile;
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION_VERSION;
      break;
    }
    case AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION_VERSION:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/version")), 1, true, HOMIE_ESP8266_VERSION);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION_OTA_ENABLED;
      break;
    case AdvertisementProgress::GlobalStep::PUB_IMPLEMENTATION_OTA_ENABLED:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/ota/enabled")), 1, true, Interface::get().getConfig().get().ota.enabled ? "true" : "false");
      if (packetId != 0) {
        if (HomieNode::nodes.size()) {  // skip if no nodes to publish
          _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_NODES;
          _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_NAME;
          _advertisementProgress.currentNodeIndex = 0;
        } else {
          _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_OTA;
        }
      }
      break;
    case AdvertisementProgress::GlobalStep::PUB_NODES:
    {
      HomieNode* node = HomieNode::nodes[_advertisementProgress.currentNodeIndex];
      std::unique_ptr<char[]> subtopic = std::unique_ptr<char[]>(new char[1 + strlen(node->getId()) + 12 + 1]);  // /id/$properties
      switch (_advertisementProgress.nodeStep) {
        case AdvertisementProgress::NodeStep::PUB_NAME:
          strcpy_P(subtopic.get(), PSTR("/"));
          strcat(subtopic.get(), node->getId());
          strcat_P(subtopic.get(), PSTR("/$name"));
          packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, node->getName());
          if (packetId != 0) _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_TYPE;
          break;
        case AdvertisementProgress::NodeStep::PUB_TYPE:
          strcpy_P(subtopic.get(), PSTR("/"));
          strcat(subtopic.get(), node->getId());
          strcat_P(subtopic.get(), PSTR("/$type"));
          packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, node->getType());
          if (packetId != 0) _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_ARRAY;
          break;
        case AdvertisementProgress::NodeStep::PUB_ARRAY:
        {
          if (!node->isRange()) {
            _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_PROPERTIES;
            break;
          }
          strcpy_P(subtopic.get(), PSTR("/"));
          strcat(subtopic.get(), node->getId());
          strcat_P(subtopic.get(), PSTR("/$array"));
          String arrayInfo;
          arrayInfo.concat(node->getLower());
          arrayInfo.concat("-");
          arrayInfo.concat(node->getUpper());

          packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, arrayInfo.c_str());
          if (packetId != 0) {
            _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_ARRAY_NODES;
            _advertisementProgress.currentArrayNodeIndex = node->getLower();
          }
          break;
        }
        case AdvertisementProgress::NodeStep::PUB_ARRAY_NODES:
        {
          String id;
          id.concat(node->getId());
          id.concat("_");
          id.concat(_advertisementProgress.currentArrayNodeIndex);
          strcpy_P(subtopic.get(), PSTR("/"));
          strcat(subtopic.get(), id.c_str());
          strcat_P(subtopic.get(), PSTR("/$name"));
          packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, id.c_str());
          if (packetId != 0) {
            if (_advertisementProgress.currentArrayNodeIndex < node->getUpper()) {
              _advertisementProgress.currentArrayNodeIndex++;
            } else {
              _advertisementProgress.currentArrayNodeIndex = node->getLower();
              _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_PROPERTIES;
            }
          }
          break;
        }
        case AdvertisementProgress::NodeStep::PUB_PROPERTIES:
        {
          strcpy_P(subtopic.get(), PSTR("/"));
          strcat(subtopic.get(), node->getId());
          strcat_P(subtopic.get(), PSTR("/$properties"));
          String properties;
          for (Property* iProperty : node->getProperties()) {
            properties.concat(iProperty->getId());
            properties.concat(",");
          }
          if (node->getProperties().size() >= 1) properties.remove(properties.length() - 1);
          packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, properties.c_str());
          if (packetId != 0) {
            if (node->getProperties().size()) {
              // There are properties of the node to be advertised
              _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_PROPERTIES_ATTRIBUTES;
              _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_NAME;
              _advertisementProgress.currentPropertyIndex = 0;
            } else {
              // No properties of the node to be advertised
              if (_advertisementProgress.currentNodeIndex < HomieNode::nodes.size() - 1) {
                // There are nodes to be advertised
                _advertisementProgress.currentNodeIndex++;
                _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_NAME;
                _advertisementProgress.currentPropertyIndex = 0;
                _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_NAME;
              } else {
                // All nodes have been advertised
                _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_OTA;
              }
            }
          }
          break;
        }
        case AdvertisementProgress::NodeStep::PUB_PROPERTIES_ATTRIBUTES:
        {
          HomieNode* node = HomieNode::nodes[_advertisementProgress.currentNodeIndex];
          Property* iProperty = node->getProperties()[_advertisementProgress.currentPropertyIndex];
          std::unique_ptr<char[]> subtopic = std::unique_ptr<char[]>(new char[1 + strlen(node->getId()) + 1 +strlen(iProperty->getId()) + 10 + 1]);  // /nodeId/propId/$settable
          switch (_advertisementProgress.propertyStep) {
            case AdvertisementProgress::PropertyStep::PUB_NAME:
              if (iProperty->getName() && (iProperty->getName()[0] != '\0')) {
                strcpy_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), node->getId());
                strcat_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), iProperty->getId());
                strcat_P(subtopic.get(), PSTR("/$name"));
                packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, iProperty->getName());
                if (packetId != 0) _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_SETTABLE;
              } else {
                _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_SETTABLE;
              }
              break;
            case AdvertisementProgress::PropertyStep::PUB_SETTABLE:
              if (iProperty->isSettable()) {
                strcpy_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), node->getId());
                strcat_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), iProperty->getId());
                strcat_P(subtopic.get(), PSTR("/$settable"));
                packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, "true");
                if (packetId != 0) _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_RETAINED;
              } else {
                _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_RETAINED;
              }
              break;
            case AdvertisementProgress::PropertyStep::PUB_RETAINED:
              if (!iProperty->isRetained()) {
                strcpy_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), node->getId());
                strcat_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), iProperty->getId());
                strcat_P(subtopic.get(), PSTR("/$retained"));
                packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, "false");
                if (packetId != 0) _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_DATATYPE;
              } else {
                _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_DATATYPE;
              }
              break;
            case AdvertisementProgress::PropertyStep::PUB_DATATYPE:
              if (iProperty->getDatatype() && (iProperty->getDatatype()[0] != '\0')) {
                strcpy_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), node->getId());
                strcat_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), iProperty->getId());
                strcat_P(subtopic.get(), PSTR("/$datatype"));
                packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, iProperty->getDatatype());
                if (packetId != 0) _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_UNIT;
              } else {
                _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_UNIT;
              }
              break;
            case AdvertisementProgress::PropertyStep::PUB_UNIT:
              if (iProperty->getUnit() && (iProperty->getUnit()[0] != '\0')) {
                strcpy_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), node->getId());
                strcat_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), iProperty->getId());
                strcat_P(subtopic.get(), PSTR("/$unit"));
                packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, iProperty->getUnit());
                if (packetId != 0) _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_FORMAT;
              } else {
                _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_FORMAT;
              }
              break;
            case AdvertisementProgress::PropertyStep::PUB_FORMAT:
            {
              bool sent = false;
              if (iProperty->getFormat() && (iProperty->getFormat()[0] != '\0')) {
                strcpy_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), node->getId());
                strcat_P(subtopic.get(), PSTR("/"));
                strcat(subtopic.get(), iProperty->getId());
                strcat_P(subtopic.get(), PSTR("/$format"));
                packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, iProperty->getFormat());
                if (packetId != 0) sent = true;
              } else {
                sent = true;
              }

              if (sent) {
                if (_advertisementProgress.currentPropertyIndex < node->getProperties().size() - 1) {
                  // Not all properties of the node have been advertised
                  _advertisementProgress.currentPropertyIndex++;
                  _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_NAME;
                } else {
                  // All properties of the node have been advertised
                  if (_advertisementProgress.currentNodeIndex < HomieNode::nodes.size() - 1) {
                    // Not all nodes have been advertised
                    _advertisementProgress.currentNodeIndex++;
                    _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_NAME;
                    _advertisementProgress.currentPropertyIndex = 0;
                    _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_NAME;
                  } else {
                    // All nodes have been advertised -> next global step
                    _advertisementProgress.currentNodeIndex = 0;
                    _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_NAME;
                    _advertisementProgress.currentPropertyIndex = 0;
                    _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_NAME;
                    _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_OTA;
                  }
                }
              }
              break;
            }
          }
        }
      }
      break;
    }
    case AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_OTA:
      packetId = Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/$implementation/ota/firmware/+")), 1);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_RESET;
      break;
    case AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_RESET:
      packetId = Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/$implementation/reset")), 1);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_CONFIG_SET;
      break;
    case AdvertisementProgress::GlobalStep::SUB_IMPLEMENTATION_CONFIG_SET:
      packetId = Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/$implementation/config/set")), 1);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_SET;
      break;
    case AdvertisementProgress::GlobalStep::SUB_SET:
      packetId = Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/+/+/set")), 2);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::SUB_BROADCAST;
      break;
    case AdvertisementProgress::GlobalStep::SUB_BROADCAST:
    {
      String broadcast_topic(Interface::get().getConfig().get().mqtt.baseTopic);
      broadcast_topic.concat("$broadcast/+");
      packetId = Interface::get().getMqttClient().subscribe(broadcast_topic.c_str(), 2);
      if (packetId != 0) _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_READY;
      break;
    }
    case AdvertisementProgress::GlobalStep::PUB_READY:
      packetId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$state")), 1, true, "ready");
      if (packetId != 0) _advertisementProgress.done = true;
      break;
  }
}

void BootNormal::_onMqttConnected() {
  _mqttDisconnectNotified = false;
  _mqttReconnectTimer.deactivate();
  _statsTimer.activate();

  Update.end();

  Interface::get().getLogger() << F("Sending initial information...") << endl;

  _advertise();
}

void BootNormal::_onMqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  Interface::get().ready = false;
  _mqttConnectNotified = false;
  _advertisementProgress.done = false;
  _advertisementProgress.globalStep = AdvertisementProgress::GlobalStep::PUB_INIT;
  _advertisementProgress.nodeStep = AdvertisementProgress::NodeStep::PUB_NAME;
  _advertisementProgress.propertyStep = AdvertisementProgress::PropertyStep::PUB_NAME;
  _advertisementProgress.currentNodeIndex = 0;
  _advertisementProgress.currentPropertyIndex = 0;
  if (!_mqttDisconnectNotified) {
    _statsTimer.deactivate();
    Interface::get().getLogger() << F("✖ MQTT disconnected, reason: ") << (int8_t)reason << endl;
    Interface::get().getLogger() << F("Triggering MQTT_DISCONNECTED event...") << endl;
    Interface::get().event.type = HomieEventType::MQTT_DISCONNECTED;
    Interface::get().event.mqttReason = reason;
    Interface::get().eventHandler(Interface::get().event);

    _mqttDisconnectNotified = true;

    if (Interface::get().flaggedForSleep) {
      _mqttOfflineMessageId = 0;
      Interface::get().getLogger() << F("Triggering READY_TO_SLEEP event...") << endl;
      Interface::get().event.type = HomieEventType::READY_TO_SLEEP;
      Interface::get().eventHandler(Interface::get().event);

      return;
    }

    _mqttConnect();
  }
  _mqttReconnectTimer.activate();
}

void BootNormal::_onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (total == 0) return;  // no empty message possible

  if (index == 0) {
    // Copy the topic
    size_t topicLength = strlen(topic);
    _mqttTopicCopy = std::unique_ptr<char[]>(new char[topicLength+1]);
    memcpy(_mqttTopicCopy.get(), topic, topicLength);
    _mqttTopicCopy.get()[topicLength] = '\0';

    // Split the topic copy on each "/"
    __splitTopic(_mqttTopicCopy.get());
  }

  // 1. Handle OTA firmware (not copied to payload buffer)
  if (__handleOTAUpdates(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 2. Fill Payload Buffer
  if (__fillPayloadBuffer(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  /* Arrived here, the payload is complete */

  // 3. handle broadcasts
  if (__handleBroadcasts(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 4.all following messages are only for this deviceId
  if (strcmp(_mqttTopicLevels.get()[0], Interface::get().getConfig().get().deviceId) != 0)
    return;

  // 5. handle reset
  if (__handleResets(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 6. handle config set
  if (__handleConfig(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;

  // 7. here, we're sure we have a node property
  if (__handleNodeProperty(_mqttTopicCopy.get(), payload, properties, len, index, total))
    return;
}

void BootNormal::_onMqttPublish(uint16_t id) {
  Interface::get().event.type = HomieEventType::MQTT_PACKET_ACKNOWLEDGED;
  Interface::get().event.packetId = id;
  Interface::get().eventHandler(Interface::get().event);

  if (Interface::get().flaggedForSleep && id == _mqttOfflineMessageId) {
    Interface::get().getLogger() << F("Offline message acknowledged. Disconnecting MQTT...") << endl;
    Interface::get().getMqttClient().disconnect();
  }
}

// _onMqttMessage Helpers

void BootNormal::__splitTopic(char* topic) {
  // split topic on each "/"
  char* afterBaseTopic = topic + strlen(Interface::get().getConfig().get().mqtt.baseTopic);

  uint8_t topicLevelsCount = 1;
  for (uint8_t i = 0; i < strlen(afterBaseTopic); i++) {
    if (afterBaseTopic[i] == '/') topicLevelsCount++;
  }

  _mqttTopicLevels = std::unique_ptr<char*[]>(new char*[topicLevelsCount]);
  _mqttTopicLevelsCount = topicLevelsCount;

  const char* delimiter = "/";
  uint8_t topicLevelIndex = 0;

  char* token = strtok(afterBaseTopic, delimiter);
  while (token != nullptr) {
    _mqttTopicLevels[topicLevelIndex++] = token;

    token = strtok(nullptr, delimiter);
  }
}

bool HomieInternals::BootNormal::__fillPayloadBuffer(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  // Reallocate Buffer everytime a new message is received
  if (_mqttPayloadBuffer == nullptr || index == 0) _mqttPayloadBuffer = std::unique_ptr<char[]>(new char[total + 1]);

  // copy payload into buffer
  memcpy(_mqttPayloadBuffer.get() + index, payload, len);

  // return if payload buffer is not complete
  if (index + len != total)
    return true;
  // terminate buffer
  _mqttPayloadBuffer.get()[total] = '\0';
  return false;
}

bool HomieInternals::BootNormal::__handleOTAUpdates(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount == 5
    && strcmp(_mqttTopicLevels.get()[0], Interface::get().getConfig().get().deviceId) == 0
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("$implementation")) == 0
    && strcmp_P(_mqttTopicLevels.get()[2], PSTR("ota")) == 0
    && strcmp_P(_mqttTopicLevels.get()[3], PSTR("firmware")) == 0
    ) {
    if (index == 0) {
      Interface::get().getLogger() << F("Receiving OTA payload") << endl;
      if (!Interface::get().getConfig().get().ota.enabled) {
        _publishOtaStatus(403);  // 403 Forbidden
        Interface::get().getLogger() << F("✖ Aborting, OTA not enabled") << endl;
        return true;
      }

      char* firmwareMd5 = _mqttTopicLevels.get()[4];
      if (!Helpers::validateMd5(firmwareMd5)) {
        _endOtaUpdate(false, UPDATE_ERROR_MD5);
        Interface::get().getLogger() << F("✖ Aborting, invalid MD5") << endl;
        return true;
      } else if (strcmp(firmwareMd5, _fwChecksum) == 0) {
        _publishOtaStatus(304);  // 304 Not Modified
        Interface::get().getLogger() << F("✖ Aborting, firmware is the same") << endl;
        return true;
      } else {
        Update.setMD5(firmwareMd5);
        _publishOtaStatus(202);
        _otaOngoing = true;

        Interface::get().getLogger() << F("↕ OTA started") << endl;
        Interface::get().getLogger() << F("Triggering OTA_STARTED event...") << endl;
        Interface::get().event.type = HomieEventType::OTA_STARTED;
        Interface::get().eventHandler(Interface::get().event);
      }
    } else if (!_otaOngoing) {
      return true; // we've not validated the checksum
    }

    // here, we need to flash the payload

    if (index == 0) {
      // Autodetect if firmware is binary or base64-encoded. ESP firmware always has a magic first byte 0xE9.
      if (*payload == 0xE9) {
        _otaIsBase64 = false;
        Interface::get().getLogger() << F("Firmware is binary") << endl;
      } else {
        // Base64-decode first two bytes. Compare decoded value against magic byte.
        char plain[2];  // need 12 bits
        base64_init_decodestate(&_otaBase64State);
        int l = base64_decode_block(payload, 2, plain, &_otaBase64State);
        if ((l == 1) && (plain[0] == 0xE9)) {
          _otaIsBase64 = true;
          _otaBase64Pads = 0;
          Interface::get().getLogger() << F("Firmware is base64-encoded") << endl;
          if (total % 4) {
            // Base64 encoded length not a multiple of 4 bytes
            _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
            return true;
          }

          // Restart base64-decoder
          base64_init_decodestate(&_otaBase64State);
        } else {
          // Bad firmware format
          _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
          return true;
        }
      }
      _otaSizeDone = 0;
      _otaSizeTotal = _otaIsBase64 ? base64_decode_expected_len(total) : total;
      bool success = Update.begin(_otaSizeTotal);
      if (!success) {
        // Detected error during begin (e.g. size == 0 or size > space)
        _endOtaUpdate(false, Update.getError());
        return true;
      }
    }

    size_t write_len;
    if (_otaIsBase64) {
      // Base64-firmware: Make sure there are no non-base64 characters in the payload.
      // libb64/cdecode.c doesn't ignore such characters if the compiler treats `char`
      // as `unsigned char`.
      size_t bin_len = 0;
      char* p = payload;
      for (size_t i = 0; i < len; i++) {
        char c = *p++;
        bool b64 = ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '+') || (c == '/');
        if (b64) {
          bin_len++;
        } else if (c == '=') {
          // Ignore "=" padding (but only at the end and only up to 2)
          if (index + i < total - 2) {
            _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
            return true;
          }
          // Note the number of pad characters at the end
          _otaBase64Pads++;
        } else {
          // Non-base64 character in firmware
          _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
          return true;
        }
      }
      if (bin_len > 0) {
        // Decode base64 payload in-place. base64_decode_block() can decode in-place,
        // except for the first two base64-characters which make one binary byte plus
        // 4 extra bits (saved in _otaBase64State). So we "manually" decode the first
        // two characters into a temporary buffer and manually merge that back into
        // the payload. This one is a little tricky, but it saves us from having to
        // dynamically allocate some 800 bytes of memory for every payload chunk.
        size_t dec_len = bin_len > 1 ? 2 : 1;
        char c;
        write_len = (size_t)base64_decode_block(payload, dec_len, &c, &_otaBase64State);
        *payload = c;

        if (bin_len > 1) {
          write_len += (size_t)base64_decode_block((const char*)payload + dec_len, bin_len - dec_len, payload + write_len, &_otaBase64State);
        }
      } else {
        write_len = 0;
      }
    } else {
      // Binary firmware
      write_len = len;
    }
    if (write_len > 0) {
      bool success = Update.write(reinterpret_cast<uint8_t*>(payload), write_len) > 0;
      if (success) {
        // Flash write successful.
        _otaSizeDone += write_len;
        if (_otaIsBase64 && (index + len == total)) {
          // Having received the last chunk of base64 encoded firmware, we can now determine
          // the real size of the binary firmware from the number of padding character ("="):
          // If we have received 1 pad character, real firmware size modulo 3 was 2.
          // If we have received 2 pad characters, real firmware size modulo 3 was 1.
          // Correct the total firmware length accordingly.
          _otaSizeTotal -= _otaBase64Pads;
        }

        String progress(_otaSizeDone);
        progress.concat(F("/"));
        progress.concat(_otaSizeTotal);
        Interface::get().getLogger() << F("Receiving OTA firmware (") << progress << F(")...") << endl;

        Interface::get().event.type = HomieEventType::OTA_PROGRESS;
        Interface::get().event.sizeDone = _otaSizeDone;
        Interface::get().event.sizeTotal = _otaSizeTotal;
        Interface::get().eventHandler(Interface::get().event);

        _publishOtaStatus(206, progress.c_str());  // 206 Partial Content

                                                   //  Done with the update?
        if (index + len == total) {
          // With base64-coded firmware, we may have provided a length off by one or two
          // to Update.begin() because the base64-coded firmware may use padding (one or
          // two "=") at the end. In case of base64, total length was adjusted above.
          // Check the real length here and ask Update::end() to skip this test.
          if ((_otaIsBase64) && (_otaSizeDone != _otaSizeTotal)) {
            _endOtaUpdate(false, UPDATE_ERROR_SIZE);
            return true;
          }
          success = Update.end(_otaIsBase64);
          _endOtaUpdate(success, Update.getError());
        }
      } else {
        // Error erasing or writing flash
        _endOtaUpdate(false, Update.getError());
      }
    }
    return true;
  }
  return false;
}

bool HomieInternals::BootNormal::__handleBroadcasts(char * topic, char * payload, const AsyncMqttClientMessageProperties & properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount == 2
    && strcmp_P(_mqttTopicLevels.get()[0], PSTR("$broadcast")) == 0
    ) {
    String broadcastLevel(_mqttTopicLevels.get()[1]);
    Interface::get().getLogger() << F("📢 Calling broadcast handler...") << endl;
    bool handled = Interface::get().broadcastHandler(broadcastLevel, _mqttPayloadBuffer.get());
    if (!handled) {
      Interface::get().getLogger() << F("The following broadcast was not handled:") << endl;
      Interface::get().getLogger() << F("  • Level: ") << broadcastLevel << endl;
      Interface::get().getLogger() << F("  • Value: ") << _mqttPayloadBuffer.get() << endl;
    }
    return true;
  }
  return false;
}

bool HomieInternals::BootNormal::__handleResets(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount == 3
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("$implementation")) == 0
    && strcmp_P(_mqttTopicLevels.get()[2], PSTR("reset")) == 0
    && strcmp_P(_mqttPayloadBuffer.get(), PSTR("true")) == 0
    ) {
    Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/reset")), 1, true, "false");
    Interface::get().getLogger() << F("Flagged for reset by network") << endl;
    Interface::get().disable = true;
    Interface::get().reset.resetFlag = true;
    return true;
  }
  return false;
}

bool HomieInternals::BootNormal::__handleConfig(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  if (
    _mqttTopicLevelsCount == 4
    && strcmp_P(_mqttTopicLevels.get()[1], PSTR("$implementation")) == 0
    && strcmp_P(_mqttTopicLevels.get()[2], PSTR("config")) == 0
    && strcmp_P(_mqttTopicLevels.get()[3], PSTR("set")) == 0
    ) {
    Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/config/set")), 1, true, "");
    if (Interface::get().getConfig().patch(_mqttPayloadBuffer.get())) {
      Interface::get().getLogger() << F("✔ Configuration updated") << endl;
      _flaggedForReboot = true;
      Interface::get().getLogger() << F("Flagged for reboot") << endl;
    } else {
      Interface::get().getLogger() << F("✖ Configuration not updated") << endl;
    }
    return true;
  }
  return false;
}

bool HomieInternals::BootNormal::__handleNodeProperty(char * topic, char * payload, const AsyncMqttClientMessageProperties& properties, size_t len, size_t index, size_t total) {
  // initialize HomieRange
  HomieRange range;
  range.isRange = false;
  range.index = 0;

  char* node = _mqttTopicLevels.get()[1];
  char* property = _mqttTopicLevels.get()[2];

  #ifdef DEBUG
    Interface::get().getLogger() << F("Recived network message for ") << homieNode->getId() << endl;
  #endif // DEBUG

  int16_t rangeSeparator = -1;
  for (uint16_t i = 0; i < strlen(node); i++) {
    if (node[i] == '_') {
      rangeSeparator = i;
      break;
    }
  }
  if (rangeSeparator != -1) {
    range.isRange = true;
    node[rangeSeparator] = '\0';
    char* rangeIndexStr = node + rangeSeparator + 1;
    String rangeIndexTest = String(rangeIndexStr);
    for (uint8_t i = 0; i < rangeIndexTest.length(); i++) {
      if (!isDigit(rangeIndexTest.charAt(i))) {
        Interface::get().getLogger() << F("Range index ") << rangeIndexStr << F(" is not valid") << endl;
        return true;
      }
    }
    range.index = rangeIndexTest.toInt();
  }

  HomieNode* homieNode = nullptr;
  homieNode = HomieNode::find(node);

  if (!homieNode) {
    Interface::get().getLogger() << F("Node ") << node << F(" not registered") << endl;
    return true;
  }

  if (homieNode->isRange()) {
    if (range.index < homieNode->getLower() || range.index > homieNode->getUpper()) {
      Interface::get().getLogger() << F("Range index ") << range.index << F(" is not within the bounds of ") << homieNode->getId() << endl;
      return true;
    }
  }

  Property* propertyObject = nullptr;
  for (Property* iProperty : homieNode->getProperties()) {
    if (strcmp(property, iProperty->getId()) == 0) {
      propertyObject = iProperty;
      break;
    }
  }

  if (!propertyObject || !propertyObject->isSettable()) {
    Interface::get().getLogger() << F("Node ") << node << F(": ") << property << F(" property not settable") << endl;
    return true;
  }

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling global input handler...") << endl;
  #endif // DEBUG
  bool handled = Interface::get().globalInputHandler(*homieNode, range, String(property), String(_mqttPayloadBuffer.get()));
  if (handled) return true;

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling node input handler...") << endl;
  #endif // DEBUG
  handled = homieNode->handleInput(range, String(property), String(_mqttPayloadBuffer.get()));
  if (handled) return true;

  #ifdef DEBUG
    Interface::get().getLogger() << F("Calling property input handler...") << endl;
  #endif // DEBUG
  handled = propertyObject->getInputHandler()(range, String(_mqttPayloadBuffer.get()));

  if (!handled) {
    Interface::get().getLogger() << F("No handlers handled the following packet:") << endl;
    Interface::get().getLogger() << F("  • Node ID: ") << node << endl;
    Interface::get().getLogger() << F("  • Is range? ");
    if (range.isRange) {
      Interface::get().getLogger() << F("yes (") << range.index << F(")") << endl;
    } else {
      Interface::get().getLogger() << F("no") << endl;
    }
    Interface::get().getLogger() << F("  • Property: ") << property << endl;
    Interface::get().getLogger() << F("  • Value: ") << _mqttPayloadBuffer.get() << endl;
  }

  return false;
}

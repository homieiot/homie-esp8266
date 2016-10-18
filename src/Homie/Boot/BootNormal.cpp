#include "BootNormal.hpp"

using namespace HomieInternals;

BootNormal::BootNormal()
: Boot("normal")
, _setupFunctionCalled(false)
, _mqttDisconnectNotified(true)
, _flaggedForOta(false)
, _flaggedForReset(false)
, _flaggedForReboot(false)
, _mqttTopic(nullptr)
, _mqttClientId(nullptr)
, _mqttWillTopic(nullptr)
, _mqttPayloadBuffer(nullptr)
, _flaggedForSleep(false)
, _mqttOfflineMessageId(0) {
  _signalQualityTimer.setInterval(SIGNAL_QUALITY_SEND_INTERVAL);
  _uptimeTimer.setInterval(UPTIME_SEND_INTERVAL);
}

BootNormal::~BootNormal() {
}

void BootNormal::_prefixMqttTopic() {
  strcpy(_mqttTopic.get(), _interface->config->get().mqtt.baseTopic);
  strcat(_mqttTopic.get(), _interface->config->get().deviceId);
}

char* BootNormal::_prefixMqttTopic(PGM_P topic) {
  _prefixMqttTopic();
  strcat_P(_mqttTopic.get(), topic);

  return _mqttTopic.get();
}

uint16_t BootNormal::_publishOtaStatus(int status, const char* info) {
  String payload(status);
  if (info) {
    payload += ' ';
    payload += info;
  }
  return _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$implementation/ota/status")), 1, true, payload.c_str());
}

uint16_t BootNormal::_publishOtaStatus_P(int status, PGM_P info) {
  return _publishOtaStatus(status, String(info).c_str());
}

void BootNormal::_wifiConnect() {
  if (_interface->led.enabled) _interface->blinker->start(LED_WIFI_DELAY);
  _interface->logger->println(F("↕ Attempting to connect to Wi-Fi..."));

  if (WiFi.getMode() != WIFI_STA) WiFi.mode(WIFI_STA);

  WiFi.hostname(_interface->config->get().deviceId);

  if (WiFi.SSID() != _interface->config->get().wifi.ssid || WiFi.psk() != _interface->config->get().wifi.password) {
    WiFi.begin(_interface->config->get().wifi.ssid, _interface->config->get().wifi.password);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  }
}

void BootNormal::_onWifiGotIp(const WiFiEventStationModeGotIP& event) {
  if (_interface->led.enabled) _interface->blinker->stop();
  _interface->logger->println(F("✔ Wi-Fi connected"));
  _interface->logger->println(F("Triggering WIFI_CONNECTED event..."));
  _interface->event.type = HomieEventType::WIFI_CONNECTED;
  _interface->event.ip = event.ip;
  _interface->event.mask = event.mask;
  _interface->event.gateway = event.gw;
  _interface->eventHandler(_interface->event);
  MDNS.begin(_interface->config->get().deviceId);

  _mqttConnect();
}

void BootNormal::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event) {
  _interface->connected = false;
  if (_interface->led.enabled) _interface->blinker->start(LED_WIFI_DELAY);
  _uptimeTimer.reset();
  _signalQualityTimer.reset();
  _interface->logger->println(F("✖ Wi-Fi disconnected"));
  _interface->logger->println(F("Triggering WIFI_DISCONNECTED event..."));
  _interface->event.type = HomieEventType::WIFI_DISCONNECTED;
  _interface->event.wifiReason = event.reason;
  _interface->eventHandler(_interface->event);

  if (!_flaggedForSleep) {
    _wifiConnect();
  }
}

void BootNormal::_mqttConnect() {
  if (_interface->led.enabled) _interface->blinker->start(LED_MQTT_DELAY);
  _interface->logger->println(F("↕ Attempting to connect to MQTT..."));
  _interface->mqttClient->connect();
}

void BootNormal::_onMqttConnected() {
  _mqttDisconnectNotified = false;
  _interface->logger->println(F("Sending initial information..."));

  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$homie")), 1, true, HOMIE_VERSION);
  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$implementation")), 1, true, "esp8266");

  for (HomieNode* iNode : HomieNode::nodes) {
    std::unique_ptr<char[]> subtopic = std::unique_ptr<char[]>(new char[1 + strlen(iNode->getId()) + 12 + 1]);  // /id/$properties
    strcpy_P(subtopic.get(), PSTR("/"));
    strcat(subtopic.get(), iNode->getId());
    strcat_P(subtopic.get(), PSTR("/$type"));
    _interface->mqttClient->publish(_prefixMqttTopic(subtopic.get()), 1, true, iNode->getType());

    strcpy_P(subtopic.get(), PSTR("/"));
    strcat(subtopic.get(), iNode->getId());
    strcat_P(subtopic.get(), PSTR("/$properties"));
    String properties;
    for (Property* iProperty : iNode->getProperties()) {
      properties.concat(iProperty->getProperty());
      if (iProperty->isRange()) {
        properties.concat("[");
        properties.concat(iProperty->getLower());
        properties.concat("-");
        properties.concat(iProperty->getUpper());
        properties.concat("]");
      }
      if (iProperty->isSettable()) properties.concat(":settable");
      properties.concat(",");
    }
    if (iNode->getProperties().size() >= 1) properties.remove(properties.length() - 1);
    _interface->mqttClient->publish(_prefixMqttTopic(subtopic.get()), 1, true, properties.c_str());
  }

  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$name")), 1, true, _interface->config->get().name);

  IPAddress localIp = WiFi.localIP();
  char localIpStr[15 + 1];
  char localIpPartStr[3 + 1];
  itoa(localIp[0], localIpPartStr, 10);
  strcpy(localIpStr, localIpPartStr);
  strcat_P(localIpStr, PSTR("."));
  itoa(localIp[1], localIpPartStr, 10);
  strcat(localIpStr, localIpPartStr);
  strcat_P(localIpStr, PSTR("."));
  itoa(localIp[2], localIpPartStr, 10);
  strcat(localIpStr, localIpPartStr);
  strcat_P(localIpStr, PSTR("."));
  itoa(localIp[3], localIpPartStr, 10);
  strcat(localIpStr, localIpPartStr);
  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$localip")), 1, true, localIpStr);

  char uptimeIntervalStr[3 + 1];
  itoa(UPTIME_SEND_INTERVAL / 1000, uptimeIntervalStr, 10);
  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$uptime/interval")), 1, true, uptimeIntervalStr);

  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$fw/name")), 1, true, _interface->firmware.name);
  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$fw/version")), 1, true, _interface->firmware.version);

  _interface->mqttClient->subscribe(_prefixMqttTopic(PSTR("/+/+/set")), 2);

  /* Implementation specific */

  char* safeConfigFile = _interface->config->getSafeConfigFile();
  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$implementation/config")), 1, true, safeConfigFile);
  free(safeConfigFile);
  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$implementation/version")), 1, true, HOMIE_ESP8266_VERSION);
  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$implementation/ota/enabled")), 1, true, _interface->config->get().ota.enabled ? "true" : "false");
  _interface->mqttClient->subscribe(_prefixMqttTopic(PSTR("/$implementation/reset")), 2);
  _interface->mqttClient->subscribe(_prefixMqttTopic(PSTR("/$implementation/config/set")), 2);

  /** Euphi: TODO #142: Homie $broadcast */
  String broadcast_topic(_interface->config->get().mqtt.baseTopic);
  broadcast_topic.concat("$broadcast/+");
  _interface->mqttClient->subscribe(broadcast_topic.c_str(), 2);

  _interface->mqttClient->subscribe(_prefixMqttTopic(PSTR("/$ota")), 2);

  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$online")), 1, true, "true");

  _interface->connected = true;
  if (_interface->led.enabled) _interface->blinker->stop();

  _interface->logger->println(F("✔ MQTT ready"));
  _interface->logger->println(F("Triggering MQTT_CONNECTED event..."));
  _interface->event.type = HomieEventType::MQTT_CONNECTED;
  _interface->eventHandler(_interface->event);

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->onReadyToOperate();
  }

  if (!_setupFunctionCalled) {
    _interface->logger->println(F("Calling setup function..."));
    _interface->setupFunction();
    _setupFunctionCalled = true;
  }
}

void BootNormal::_onMqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  _interface->connected = false;
  if (!_mqttDisconnectNotified) {
    _uptimeTimer.reset();
    _signalQualityTimer.reset();
    _interface->logger->println(F("✖ MQTT disconnected"));
    _interface->logger->println(F("Triggering MQTT_DISCONNECTED event..."));
    _interface->event.type = HomieEventType::MQTT_DISCONNECTED;
    _interface->event.mqttReason = reason;
    _interface->eventHandler(_interface->event);
    if (_flaggedForSleep) {
      _interface->logger->println(F("Triggering READY_TO_SLEEP event..."));
      _interface->event.type = HomieEventType::READY_TO_SLEEP;
      _interface->eventHandler(_interface->event);
    }
    _mqttDisconnectNotified = true;
  }
  if (!_flaggedForSleep) {
    _mqttConnect();
  }
}

void BootNormal::_onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (total == 0) return;  // no empty message possible

  HomieRange range;
  range.isRange = false;
  range.index = 0;

  // Check for Broadcast first (it does not contain device-id)
  char* broadcast_topic = topic + strlen(_interface->config->get().mqtt.baseTopic);
  // Skip devices/${id}/ --- +1 for /
  char* device_topic = broadcast_topic + strlen(_interface->config->get().deviceId) + 1;

  // 1. Handle OTA firmware (not copied to payload buffer)
  if (strcmp_P(device_topic, PSTR("$implementation/ota/firmware")) == 0) {  // If this is the OTA firmware
    if (_flaggedForOta) {
      if (index == 0) {
        Update.begin(total);
        _interface->logger->println(F("OTA started"));
        _interface->logger->println(F("Triggering OTA_STARTED event..."));
        _interface->event.type = HomieEventType::OTA_STARTED;
        _interface->eventHandler(_interface->event);
      }
      String progress(index + len);
      progress += '/';
      progress += total;
      _publishOtaStatus(206, progress.c_str());  // 206 Partial Content
      _interface->logger->print(F("Receiving OTA firmware ("));
      _interface->logger->print(progress.c_str());
      _interface->logger->println(F(")..."));

      Update.write(reinterpret_cast<uint8_t*>(payload), len);

      if (index + len == total) {
        bool success = Update.end();

        if (success) {
          _interface->logger->println(F("✔ OTA success"));
          _interface->logger->println(F("Triggering OTA_SUCCESSFUL event..."));
          _interface->event.type = HomieEventType::OTA_SUCCESSFUL;
          _interface->eventHandler(_interface->event);
          _publishOtaStatus(200);  // 200 OK
          _flaggedForReboot = true;
        } else {
          _interface->logger->println(F("✖ OTA failed"));
          _interface->logger->println(F("Triggering OTA_FAILED event..."));
          _interface->event.type = HomieEventType::OTA_FAILED;
          _interface->eventHandler(_interface->event);
          int status;
          String info;
          switch (Update.getError()) {
            case UPDATE_ERROR_SIZE:               // new firmware size is zero
            case UPDATE_ERROR_MAGIC_BYTE:         // new firmware does not have 0xE9 in first byte
            case UPDATE_ERROR_NEW_FLASH_CONFIG:   // bad new flash config (does not match flash ID)
              status = 400;  // 400 Bad Request
              info = PSTR("BAD_FIRMWARE");
              break;
            case UPDATE_ERROR_MD5:
              status = 400;  // 400 Bad Request
              info = PSTR("BAD_CHECKSUM");
              break;
            case UPDATE_ERROR_SPACE:
              status = 400;  // 400 Bad Request
              info = PSTR("NOT_ENOUGH_SPACE");
              break;
            case UPDATE_ERROR_WRITE:
            case UPDATE_ERROR_ERASE:
            case UPDATE_ERROR_READ:
              status = 500;  // 500 Internal Server Error
              info = PSTR("FLASH_ERROR");
              break;
            default:
              status = 500;  // 500 Internal Server Error
              info = PSTR("INTERNAL_ERROR ") + Update.getError();
              break;
          }
          _publishOtaStatus(status, info.c_str());
        }

        _flaggedForOta = false;
        _interface->mqttClient->unsubscribe(_prefixMqttTopic(PSTR("/$implementation/ota/firmware")));
      }
    } else {
      _interface->logger->print(F("Receiving OTA firmware but not requested, skipping..."));
      if (_interface->config->get().ota.enabled)
        _publishOtaStatus(400, PSTR("NOT_REQUESTED"));
      else
        _publishOtaStatus(403);  // 403 Forbidden
    }
    return;
  }

  // 2. Fill Payload Buffer

  // Reallocate Buffer everytime a new message is received
  if (_mqttPayloadBuffer == nullptr || index == 0) _mqttPayloadBuffer = std::unique_ptr<char[]>(new char[total + 1]);

  // TODO(euphi): Check if buffer size matches payload length
  memcpy(_mqttPayloadBuffer.get() + index, payload, len);

  if (index + len != total) return;  // return if payload buffer is not complete
  _mqttPayloadBuffer.get()[total] = '\0';

  // 3. Special Functions: $broadcast
  /** TODO(euphi): Homie $broadcast */
  if (strncmp(broadcast_topic, "$broadcast", 10) == 0) {
    broadcast_topic += sizeof("$broadcast");  // move pointer to second char after $broadcast (sizeof counts the \0)
    String broadcastLevel(broadcast_topic);
    _interface->logger->println(F("Calling broadcast handler..."));
    bool handled = _interface->broadcastHandler(broadcastLevel, _mqttPayloadBuffer.get());
    if (!handled) {
      _interface->logger->println(F("The following broadcast was not handled:"));
      _interface->logger->print(F("  • Level: "));
      _interface->logger->println(broadcastLevel);
      _interface->logger->print(F("  • Value: "));
      _interface->logger->println(_mqttPayloadBuffer.get());
    }
    return;
  }

  // 4. Special Functions: $ota
  if (strcmp_P(device_topic, PSTR("$ota")) == 0) {  // If this is the $ota announcement
    if (_interface->config->get().ota.enabled) {
      if (strcmp(_mqttPayloadBuffer.get(), _interface->firmware.version) != 0) {
        _interface->logger->print(F("? OTA available (version "));
        _interface->logger->print(_mqttPayloadBuffer.get());
        _interface->logger->println(F(")"));

        _interface->logger->println(F("Subscribing to OTA firmware..."));
        _interface->mqttClient->subscribe(_prefixMqttTopic(PSTR("/$implementation/ota/firmware")), 0);
        _flaggedForOta = true;
        _publishOtaStatus(202);  // 202 Accepted
      } else {
        _publishOtaStatus(304);  // 304 Not Modified
      }
    } else {
      _publishOtaStatus(403);  // 403 Forbidden
    }
    return;
  }

  // 5. Special Functions: $reset
  if (strcmp_P(device_topic, PSTR("$implementation/reset")) == 0 && strcmp(_mqttPayloadBuffer.get(), "true") == 0) {
    _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$implementation/reset")), 1, true, "false");
    _flaggedForReset = true;
    _interface->logger->println(F("Flagged for reset by network"));
    return;
  }

  // 6. Special Functions set $config
  if (strcmp_P(device_topic, PSTR("$implementation/config/set")) == 0) {
    if (_interface->config->patch(_mqttPayloadBuffer.get())) {
      _interface->logger->println(F("✔ Configuration updated"));
      _flaggedForReboot = true;
      _interface->logger->println(F("Flagged for reboot"));
    } else {
      _interface->logger->println(F("✖ Configuration not updated"));
    }
    return;
  }


  // 7. Determine specific Node

  // Implicit node properties
  device_topic[strlen(device_topic) - 4] = '\0';  // Remove /set
  uint16_t separator = 0;
  for (uint16_t i = 0; i < strlen(device_topic); i++) {
    if (device_topic[i] == '/') {
      separator = i;
      break;
    }
  }
  char* node = device_topic;
  node[separator] = '\0';
  char* property = device_topic + separator + 1;
  HomieNode* homieNode = HomieNode::find(node);
  if (!homieNode) {
    _interface->logger->print(F("Node "));
    _interface->logger->print(node);
    _interface->logger->println(F(" not registered"));
    return;
  }

  int16_t rangeSeparator = -1;
  for (uint16_t i = 0; i < strlen(property); i++) {
    if (property[i] == '_') {
      rangeSeparator = i;
      break;
    }
  }
  if (rangeSeparator != -1) {
    range.isRange = true;
    property[rangeSeparator] = '\0';
    char* rangeIndexStr = property + rangeSeparator + 1;
    String rangeIndexTest = String(rangeIndexStr);
    for (uint8_t i = 0; i < rangeIndexTest.length(); i++) {
      if (!isDigit(rangeIndexTest.charAt(i))) {
        _interface->logger->print(F("Range index "));
        _interface->logger->print(rangeIndexStr);
        _interface->logger->println(F(" is not valid"));
        return;
      }
    }
    range.index = rangeIndexTest.toInt();
  }

  Property* propertyObject = nullptr;
  for (Property* iProperty : homieNode->getProperties()) {
    if (range.isRange) {
      if (iProperty->isRange() && strcmp(property, iProperty->getProperty()) == 0) {
        if (range.index >= iProperty->getLower() && range.index <= iProperty->getUpper()) {
          propertyObject = iProperty;
          break;
        } else {
          _interface->logger->print(F("Range index "));
          _interface->logger->print(range.index);
          _interface->logger->print(F(" is not within the bounds of "));
          _interface->logger->println(property);
          return;
        }
      }
    } else if (strcmp(property, iProperty->getProperty()) == 0) {
      propertyObject = iProperty;
      break;
    }
  }

  if (!propertyObject || !propertyObject->isSettable()) {
    _interface->logger->print(F("Node "));
    _interface->logger->print(node);
    _interface->logger->print(F(":"));
    _interface->logger->println(property);
    _interface->logger->println(F(" property not settable"));
    return;
  }

  _interface->logger->println(F("Calling global input handler..."));
  bool handled = _interface->globalInputHandler(*homieNode, String(property), range, String(_mqttPayloadBuffer.get()));
  if (handled) return;

  _interface->logger->println(F("Calling node input handler..."));
  handled = homieNode->handleInput(String(property), range, String(_mqttPayloadBuffer.get()));
  if (handled) return;

  _interface->logger->println(F("Calling property input handler..."));
  handled = propertyObject->getInputHandler()(range, String(_mqttPayloadBuffer.get()));

  if (!handled) {
    _interface->logger->println(F("No handlers handled the following packet:"));
    _interface->logger->print(F("  • Node ID: "));
    _interface->logger->println(node);
    _interface->logger->print(F("  • Property: "));
    _interface->logger->println(property);
    _interface->logger->print(F("  • Is range? "));
    if (range.isRange) {
      _interface->logger->print(F("yes ("));
      _interface->logger->print(range.index);
      _interface->logger->println(F(")"));
    } else {
      _interface->logger->println(F("no"));
    }
    _interface->logger->print(F("  • Value: "));
    _interface->logger->println(_mqttPayloadBuffer.get());
  }
}

void BootNormal::_onMqttPublish(uint16_t id) {
  _interface->logger->print(F("Triggering MQTT_PACKET_ACKNOWLEDGED event (packetId "));
  _interface->logger->print(id);
  _interface->logger->println(F(")..."));
  _interface->event.type = HomieEventType::MQTT_PACKET_ACKNOWLEDGED;
  _interface->event.packetId = id;
  _interface->eventHandler(_interface->event);

  if (_flaggedForSleep && id == _mqttOfflineMessageId) {
    _interface->logger->println(F("Offline message acknowledged. Disconnecting MQTT..."));
    _interface->mqttClient->disconnect();
  }
}

void BootNormal::_handleReset() {
  if (_interface->reset.enabled) {
    _resetDebouncer.update();

    if (_resetDebouncer.read() == _interface->reset.triggerState) {
      _flaggedForReset = true;
      _interface->logger->println(F("Flagged for reset by pin"));
    }
  }

  if (_interface->reset.userFunction()) {
    _flaggedForReset = true;
    _interface->logger->println(F("Flagged for reset by function"));
  }
}

void BootNormal::setup() {
  Boot::setup();

  Update.runAsync(true);

  if (_interface->led.enabled) _interface->blinker->start(LED_WIFI_DELAY);

  // Generate topic buffer
  size_t baseTopicLength = strlen(_interface->config->get().mqtt.baseTopic) + strlen(_interface->config->get().deviceId);
  size_t longestSubtopicLength = 29 + 1;  // /$implementation/ota/firmware
  for (HomieNode* iNode : HomieNode::nodes) {
    size_t nodeMaxTopicLength = 1 + strlen(iNode->getId()) + 12 + 1;  // /id/$properties
    if (nodeMaxTopicLength > longestSubtopicLength) longestSubtopicLength = nodeMaxTopicLength;

    for (Property* iProperty : iNode->getProperties()) {
      size_t propertyMaxTopicLength = 1 + strlen(iNode->getId()) + 1 + strlen(iProperty->getProperty()) + 1;
      if (iProperty->isSettable()) propertyMaxTopicLength += 4;  // /set

      if (propertyMaxTopicLength > longestSubtopicLength) longestSubtopicLength = propertyMaxTopicLength;
    }
  }
  _mqttTopic = std::unique_ptr<char[]>(new char[baseTopicLength + longestSubtopicLength]);

  _wifiGotIpHandler = WiFi.onStationModeGotIP(std::bind(&BootNormal::_onWifiGotIp, this, std::placeholders::_1));
  _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&BootNormal::_onWifiDisconnected, this, std::placeholders::_1));

  _interface->mqttClient->onConnect(std::bind(&BootNormal::_onMqttConnected, this));
  _interface->mqttClient->onDisconnect(std::bind(&BootNormal::_onMqttDisconnected, this, std::placeholders::_1));
  _interface->mqttClient->onMessage(std::bind(&BootNormal::_onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  _interface->mqttClient->onPublish(std::bind(&BootNormal::_onMqttPublish, this, std::placeholders::_1));

  _interface->mqttClient->setServer(_interface->config->get().mqtt.server.host, _interface->config->get().mqtt.server.port);
  _interface->mqttClient->setKeepAlive(10).setMaxTopicLength(MAX_MQTT_TOPIC_LENGTH);
  _mqttClientId = std::unique_ptr<char[]>(new char[strlen(_interface->brand) + 1 + strlen(_interface->config->get().deviceId) + 1]);
  strcpy(_mqttClientId.get(), _interface->brand);
  strcat_P(_mqttClientId.get(), PSTR("-"));
  strcat(_mqttClientId.get(), _interface->config->get().deviceId);
  _interface->mqttClient->setClientId(_mqttClientId.get());
  char* mqttWillTopic = _prefixMqttTopic(PSTR("/$online"));
  _mqttWillTopic = std::unique_ptr<char[]>(new char[strlen(mqttWillTopic) + 1]);
  memcpy(_mqttWillTopic.get(), mqttWillTopic, strlen(mqttWillTopic) + 1);
  _interface->mqttClient->setWill(_mqttWillTopic.get(), 1, true, "false");

  if (_interface->config->get().mqtt.auth) _interface->mqttClient->setCredentials(_interface->config->get().mqtt.username, _interface->config->get().mqtt.password);


  if (_interface->reset.enabled) {
    pinMode(_interface->reset.triggerPin, INPUT_PULLUP);

    _resetDebouncer.attach(_interface->reset.triggerPin);
    _resetDebouncer.interval(_interface->reset.triggerTime);
  }

  _interface->config->log();

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->setup();
  }

  _wifiConnect();
}

void BootNormal::loop() {
  Boot::loop();

  _handleReset();

  if (_flaggedForReset && _interface->reset.able) {
    _interface->logger->println(F("Device is in a resettable state"));
    _interface->config->erase();
    _interface->logger->println(F("Configuration erased"));

    _interface->logger->println(F("Triggering ABOUT_TO_RESET event..."));
    _interface->event.type = HomieEventType::ABOUT_TO_RESET;
    _interface->eventHandler(_interface->event);

    _interface->logger->println(F("↻ Rebooting into config mode..."));
    Serial.flush();
    ESP.restart();
  }

  if (_flaggedForReboot && _interface->reset.able) {
    _interface->logger->println(F("Device is in a resettable state"));

    _interface->logger->println(F("↻ Rebooting..."));
    Serial.flush();
    ESP.restart();
  }

  if (!_interface->connected) return;

  if (_signalQualityTimer.check()) {
    uint8_t quality = Helpers::rssiToPercentage(WiFi.RSSI());

    char qualityStr[3 + 1];
    itoa(quality, qualityStr, 10);

    _interface->logger->print(F("Sending Wi-Fi signal quality ("));
    _interface->logger->print(qualityStr);
    _interface->logger->println(F("%)..."));

    _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$signal")), 1, true, qualityStr);
    _signalQualityTimer.tick();
  }

  if (_uptimeTimer.check()) {
    _uptime.update();

    char uptimeStr[10 + 1];
    itoa(_uptime.getSeconds(), uptimeStr, 10);

    _interface->logger->print(F("Sending uptime ("));
    _interface->logger->print(_uptime.getSeconds());
    _interface->logger->println(F("s)..."));

    _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$uptime/value")), 1, true, uptimeStr);
    _uptimeTimer.tick();
  }

  _interface->loopFunction();

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->loop();
  }
}

void BootNormal::prepareToSleep() {
  _interface->logger->println(F("Sending offline message..."));
  _flaggedForSleep = true;
  _mqttOfflineMessageId = _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$online")), 1, true, "false");
}

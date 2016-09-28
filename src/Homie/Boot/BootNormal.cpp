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

void BootNormal::_wifiConnect() {
  if (_interface->led.enabled) _interface->blinker->start(LED_WIFI_DELAY);
  _interface->logger->logln(F("↕ Attempting to connect to Wi-Fi..."));

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
  _interface->logger->logln(F("✔ Wi-Fi connected"));
  _interface->logger->logln(F("Triggering WIFI_CONNECTED event..."));
  _interface->eventHandler(HomieEvent::WIFI_CONNECTED);
  MDNS.begin(_interface->config->get().deviceId);

  _mqttConnect();
}

void BootNormal::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event) {
  _interface->connected = false;
  if (_interface->led.enabled) _interface->blinker->start(LED_WIFI_DELAY);
  _uptimeTimer.reset();
  _signalQualityTimer.reset();
  _interface->logger->logln(F("✖ Wi-Fi disconnected"));
  _interface->logger->logln(F("Triggering WIFI_DISCONNECTED event..."));
  _interface->eventHandler(HomieEvent::WIFI_DISCONNECTED);

  if (!_flaggedForSleep) {
    _wifiConnect();
  }
}

void BootNormal::_mqttConnect() {
  if (_interface->led.enabled) _interface->blinker->start(LED_MQTT_DELAY);
  _interface->logger->logln(F("↕ Attempting to connect to MQTT..."));
  _interface->mqttClient->connect();
}

void BootNormal::_onMqttConnected() {
  _mqttDisconnectNotified = false;
  _interface->logger->logln(F("Sending initial information..."));

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

  Serial.printf("Subscribing to [%s].\n",broadcast_topic.c_str());

  _interface->mqttClient->subscribe(broadcast_topic.c_str(), 2);

  if (_interface->config->get().ota.enabled) {
    _interface->mqttClient->subscribe(_prefixMqttTopic(PSTR("/$ota")), 2);
  }

  _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$online")), 1, true, "true");

  _interface->connected = true;
  if (_interface->led.enabled) _interface->blinker->stop();

  _interface->logger->logln(F("✔ MQTT ready"));
  _interface->logger->logln(F("Triggering MQTT_CONNECTED event..."));
  _interface->eventHandler(HomieEvent::MQTT_CONNECTED);

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->onReadyToOperate();
  }

  if (!_setupFunctionCalled) {
    _interface->logger->logln(F("Calling setup function..."));
    _interface->setupFunction();
    _setupFunctionCalled = true;
  }
}

void BootNormal::_onMqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  _interface->connected = false;
  if (!_mqttDisconnectNotified) {
    _uptimeTimer.reset();
    _signalQualityTimer.reset();
    _interface->logger->logln(F("✖ MQTT disconnected"));
    _interface->logger->logln(F("Triggering MQTT_DISCONNECTED event..."));
    _interface->eventHandler(HomieEvent::MQTT_DISCONNECTED);
    if (_flaggedForSleep) {
      _interface->logger->logln(F("Triggering READY_FOR_SLEEP event..."));
      _interface->eventHandler(HomieEvent::READY_FOR_SLEEP);
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

  //Check for Broadcast first (it does not contain device-id)
  char* broadcast_topic = topic + strlen(_interface->config->get().mqtt.baseTopic);
  // Skip devices/${id}/ --- +1 for /
  char* device_topic = broadcast_topic + strlen(_interface->config->get().deviceId) + 1;

  Serial.printf("%d: Received mqtt topic %s:\n\t%s\n\t\%s\n\t\t%s\n", millis(), topic, broadcast_topic,device_topic, payload);
  if (strcmp_P(device_topic, PSTR("$implementation/ota/payload")) == 0) {  // If this is the $ota payload
    if (_flaggedForOta) {
      if (index == 0) {
        Update.begin(total);
        _interface->logger->logln(F("OTA started"));
        _interface->logger->logln(F("Triggering OTA_STARTED event..."));
        _interface->eventHandler(HomieEvent::OTA_STARTED);
      }
      _interface->logger->log(F("Receiving OTA payload ("));
      _interface->logger->log(index + len);
      _interface->logger->log(F("/"));
      _interface->logger->log(total);
      _interface->logger->logln(F(")..."));

      Update.write(reinterpret_cast<uint8_t*>(payload), len);

      if (index + len == total) {
        bool success = Update.end();

        if (success) {
          _interface->logger->logln(F("✔ OTA success"));
          _interface->logger->logln(F("Triggering OTA_SUCCESSFUL event..."));
          _interface->eventHandler(HomieEvent::OTA_SUCCESSFUL);
          _flaggedForReboot = true;
        } else {
          _interface->logger->logln(F("✖ OTA failed"));
          _interface->logger->logln(F("Triggering OTA_FAILED event..."));
          _interface->eventHandler(HomieEvent::OTA_FAILED);
        }

        _flaggedForOta = false;
        _interface->mqttClient->unsubscribe(_prefixMqttTopic(PSTR("/$implementation/ota/payload")));
      }
    } else {
      _interface->logger->log(F("Receiving OTA payload but not requested, skipping..."));
    }
    return;
  }

  // Reallocate Buffer everytime a new message is received
  if (_mqttPayloadBuffer == nullptr || index == 0) _mqttPayloadBuffer = std::unique_ptr<char[]>(new char[total + 1]);

  //TODO: Check if buffer size matches payload length
  memcpy(_mqttPayloadBuffer.get() + index, payload, len);

  if (index + len != total) return;
  _mqttPayloadBuffer.get()[total] = '\0';

  /** Euphi: TODO #142: Homie $broadcast */
  if (strncmp(broadcast_topic, "$broadcast", 10) == 0) {
	  broadcast_topic += sizeof("$broadcast"); // move pointer to second char after $broadcast (sizeof counts the \0)
	  String broadcastlevel(broadcast_topic);
	  Serial.printf("Received Broadcast with level %s: %s.\n", broadcastlevel.c_str(), payload);
	  _interface->logger->logln(F("Calling global input handler for broadcast..."));
	  bool handled = _interface->globalInputHandler(String("$broadcast"), broadcastlevel, range, _mqttPayloadBuffer.get());
	  if (!handled) _interface->logger->logln(F("Broadcast not handled"));
	  return;
  }


  if (strcmp_P(device_topic, PSTR("$ota")) == 0) {  // If this is the $ota announcement
    if (strcmp(_mqttPayloadBuffer.get(), _interface->firmware.version) != 0) {
      _interface->logger->log(F("✴ OTA available (version "));
      _interface->logger->log(_mqttPayloadBuffer.get());
      _interface->logger->logln(F(")"));

      _interface->logger->logln(F("Subscribing to OTA payload..."));
      _interface->mqttClient->subscribe(_prefixMqttTopic(PSTR("/$implementation/ota/payload")), 0);
      _flaggedForOta = true;
    }

    return;
  }

  if (strcmp_P(device_topic, PSTR("$implementation/reset")) == 0 && strcmp(_mqttPayloadBuffer.get(), "true") == 0) {
    _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$implementation/reset")), 1, true, "false");
    _flaggedForReset = true;
    _interface->logger->logln(F("Flagged for reset by network"));
    return;
  }

  if (strcmp_P(device_topic, PSTR("$implementation/config/set")) == 0) {
    if (_interface->config->patch(_mqttPayloadBuffer.get())) {
      _interface->logger->logln(F("✔ Configuration updated"));
      _flaggedForReboot = true;
      _interface->logger->logln(F("Flagged for reboot"));
    } else {
      _interface->logger->logln(F("✖ Configuration not updated"));
    }
    return;
  }


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
    _interface->logger->log(F("Node "));
    _interface->logger->log(node);
    _interface->logger->logln(F(" not registered"));
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
        _interface->logger->log(F("Range index "));
        _interface->logger->log(rangeIndexStr);
        _interface->logger->logln(F(" is not valid"));
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
          _interface->logger->log(F("Range index "));
          _interface->logger->log(range.index);
          _interface->logger->log(F(" is not within the bounds of "));
          _interface->logger->logln(property);
          return;
        }
      }
    } else if (strcmp(property, iProperty->getProperty()) == 0) {
      propertyObject = iProperty;
      break;
    }
  }

  if (!propertyObject || !propertyObject->isSettable()) {
    _interface->logger->log(F("Node "));
    _interface->logger->log(node);
    _interface->logger->log(F(":"));
    _interface->logger->logln(property);
    _interface->logger->log(F(" property not settable"));
    return;
  }

  _interface->logger->logln(F("Calling global input handler..."));
  bool handled = _interface->globalInputHandler(String(node), String(property), range, String(_mqttPayloadBuffer.get()));
  if (handled) return;

  _interface->logger->logln(F("Calling node input handler..."));
  handled = homieNode->handleInput(String(property), range, String(_mqttPayloadBuffer.get()));
  if (handled) return;

  _interface->logger->logln(F("Calling property input handler..."));
  handled = propertyObject->getInputHandler()(range, String(_mqttPayloadBuffer.get()));

  if (!handled) {
    _interface->logger->logln(F("No handlers handled the following packet:"));
    _interface->logger->log(F("  • Node ID: "));
    _interface->logger->logln(node);
    _interface->logger->log(F("  • Property: "));
    _interface->logger->logln(property);
    _interface->logger->log(F("  • Is range? "));
    if (range.isRange) {
      _interface->logger->log(F("yes ("));
      _interface->logger->log(range.index);
      _interface->logger->logln(F(")"));
    } else {
      _interface->logger->logln(F("no"));
    }
    _interface->logger->log(F("  • Value: "));
    _interface->logger->logln(_mqttPayloadBuffer.get());
  }
}

void BootNormal::_onMqttPublish(uint16_t id) {
  if (_flaggedForSleep && id == _mqttOfflineMessageId) {
    _interface->logger->logln(F("Offline message acknowledged.  Disconnecting MQTT..."));
    _interface->mqttClient->disconnect();
  }
}

void BootNormal::_handleReset() {
  if (_interface->reset.enabled) {
    _resetDebouncer.update();

    if (_resetDebouncer.read() == _interface->reset.triggerState) {
      _flaggedForReset = true;
      _interface->logger->logln(F("Flagged for reset by pin"));
    }
  }

  if (_interface->reset.userFunction()) {
    _flaggedForReset = true;
    _interface->logger->logln(F("Flagged for reset by function"));
  }
}

void BootNormal::setup() {
  Boot::setup();

  Update.runAsync(true);

  if (_interface->led.enabled) _interface->blinker->start(LED_WIFI_DELAY);

  // Generate topic buffer
  size_t baseTopicLength = strlen(_interface->config->get().mqtt.baseTopic) + strlen(_interface->config->get().deviceId);
  size_t longestSubtopicLength = 28 + 1;  // /$implementation/ota/enabled
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
  _interface->mqttClient->setKeepAlive(10);
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
    _interface->logger->logln(F("Device is in a resettable state"));
    _interface->config->erase();
    _interface->logger->logln(F("Configuration erased"));

    _interface->logger->logln(F("Triggering ABOUT_TO_RESET event..."));
    _interface->eventHandler(HomieEvent::ABOUT_TO_RESET);

    _interface->logger->logln(F("↻ Rebooting into config mode..."));
    _interface->logger->flush();
    ESP.restart();
  }

  if (_flaggedForReboot && _interface->reset.able) {
    _interface->logger->logln(F("Device is in a resettable state"));

    _interface->logger->logln(F("↻ Rebooting..."));
    _interface->logger->flush();
    ESP.restart();
  }

  if (!_interface->connected) return;

  if (_signalQualityTimer.check()) {
    uint8_t quality = Helpers::rssiToPercentage(WiFi.RSSI());

    char qualityStr[3 + 1];
    itoa(quality, qualityStr, 10);

    _interface->logger->log(F("Sending Wi-Fi signal quality ("));
    _interface->logger->log(qualityStr);
    _interface->logger->logln(F("%)..."));

    _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$signal")), 1, true, qualityStr);
    _signalQualityTimer.tick();
  }

  if (_uptimeTimer.check()) {
    _uptime.update();

    char uptimeStr[10 + 1];
    itoa(_uptime.getSeconds(), uptimeStr, 10);

    _interface->logger->log(F("Sending uptime ("));
    _interface->logger->log(_uptime.getSeconds());
    _interface->logger->logln(F("s)..."));

    _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$uptime/value")), 1, true, uptimeStr);
    _uptimeTimer.tick();
  }

  _interface->loopFunction();

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->loop();
  }
}

void BootNormal::prepareForSleep() {
  _interface->logger->logln(F("Sending offline message..."));
  _flaggedForSleep = true;
  _mqttOfflineMessageId = _interface->mqttClient->publish(_prefixMqttTopic(PSTR("/$online")), 1, true, "false");
}

#include "BootNormal.hpp"

using namespace HomieInternals;

BootNormal::BootNormal()
: Boot("normal")
, _setupFunctionCalled(false)
, _mqttConnectNotified(false)
, _mqttDisconnectNotified(true)
, _otaVersion {'\0'}
, _flaggedForOta(false)
, _flaggedForReset(false)
{
  _mqttReconnectTimer.setInterval(MQTT_RECONNECT_INTERVAL);
  _signalQualityTimer.setInterval(SIGNAL_QUALITY_SEND_INTERVAL);
  _uptimeTimer.setInterval(UPTIME_SEND_INTERVAL);
}

BootNormal::~BootNormal() {
}

void BootNormal::_fillMqttTopic(PGM_P topic) {
  strcpy(_interface->mqttClient->getTopicBuffer(), _interface->config->get().mqtt.baseTopic);
  strcat(_interface->mqttClient->getTopicBuffer(), _interface->config->get().deviceId);
  strcat_P(_interface->mqttClient->getTopicBuffer(), topic);
}

bool BootNormal::_publishRetainedOrFail(const char* message) {
  if (!_interface->mqttClient->publish(message, true)) {
    _interface->mqttClient->disconnect();
    _interface->logger->logln(F(" Failed"));
    return false;
  }

  return true;
}

bool BootNormal::_subscribe1OrFail() {
  if (!_interface->mqttClient->subscribe(1)) {
    _interface->mqttClient->disconnect();
    _interface->logger->logln(F(" Failed"));
    return false;
  }

  return true;
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

void BootNormal::_onWifiGotIp(const WiFiEventStationModeGotIP& event)
{
  if (_interface->led.enabled) _interface->blinker->stop();
  _interface->logger->logln(F("✔ Wi-Fi connected"));
  _interface->logger->logln(F("Triggering HOMIE_WIFI_CONNECTED event..."));
  _interface->eventHandler(HOMIE_WIFI_CONNECTED);
  MDNS.begin(_interface->config->get().deviceId);
  // Serial.println(event.ip);
}

void BootNormal::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event)
{
  _uptimeTimer.reset();
  _signalQualityTimer.reset();
  _interface->logger->logln(F("✖ Wi-Fi disconnected"));
  _interface->logger->logln(F("Triggering HOMIE_WIFI_DISCONNECTED event..."));
  _interface->eventHandler(HOMIE_WIFI_DISCONNECTED);

  _wifiConnect();
  // Serial.println(event.reason);
}

void BootNormal::_mqttConnect() {
  String host = _interface->config->get().mqtt.server.host;
  uint16_t port = _interface->config->get().mqtt.server.port;
  if (_interface->config->get().mqtt.server.mdns.enabled) {
    _interface->logger->log(F("Querying mDNS service "));
    _interface->logger->logln(_interface->config->get().mqtt.server.mdns.service);
    MdnsQueryResult result = Helpers::mdnsQuery(_interface->config->get().mqtt.server.mdns.service);
    if (result.success) {
      host = result.ip.toString();
      port = result.port;
      _interface->logger->log(F("✔ "));
      _interface->logger->log(F(" service found at "));
      _interface->logger->log(host.c_str());
      _interface->logger->log(F(":"));
      _interface->logger->logln(port);
    } else {
      _interface->logger->logln(F("✖ Service not found"));
      return;
    }
  }

  _interface->mqttClient->setServer(host.c_str(), port, _interface->config->get().mqtt.server.ssl.fingerprint);
  _interface->mqttClient->setCallback(std::bind(&BootNormal::_mqttCallback, this, std::placeholders::_1, std::placeholders::_2));

  char clientId[MAX_WIFI_SSID_LENGTH];
  strcpy(clientId, _interface->brand);
  strcat_P(clientId, PSTR("-"));
  strcat(clientId, _interface->config->get().deviceId);

  _fillMqttTopic(PSTR("/$online"));
  if (_interface->mqttClient->connect(clientId, "false", 2, true, _interface->config->get().mqtt.auth, _interface->config->get().mqtt.username, _interface->config->get().mqtt.password)) {
    _interface->logger->logln(F("Connected"));
    _mqttSetup();
  } else {
    _interface->logger->log(F("✖ Cannot connect, reason: "));
    switch (_interface->mqttClient->getState()) {
      case MQTT_CONNECTION_TIMEOUT:
        _interface->logger->logln(F("MQTT_CONNECTION_TIMEOUT"));
        break;
      case MQTT_CONNECTION_LOST:
        _interface->logger->logln(F("MQTT_CONNECTION_LOST"));
        break;
      case MQTT_CONNECT_FAILED:
        _interface->logger->logln(F("MQTT_CONNECT_FAILED"));
        break;
      case MQTT_DISCONNECTED:
        _interface->logger->logln(F("MQTT_DISCONNECTED"));
        break;
      case MQTT_CONNECTED:
        _interface->logger->logln(F("MQTT_CONNECTED (?)"));
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        _interface->logger->logln(F("MQTT_CONNECT_BAD_PROTOCOL"));
        break;
      case MQTT_CONNECT_BAD_CLIENT_ID:
        _interface->logger->logln(F("MQTT_CONNECT_BAD_CLIENT_ID"));
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        _interface->logger->logln(F("MQTT_CONNECT_UNAVAILABLE"));
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        _interface->logger->logln(F("MQTT_CONNECT_BAD_CREDENTIALS"));
        break;
      case MQTT_CONNECT_UNAUTHORIZED:
        _interface->logger->logln(F("MQTT_CONNECT_UNAUTHORIZED"));
        break;
      default:
        _interface->logger->logln(F("UNKNOWN"));
    }
  }
}

void BootNormal::_mqttSetup() {
  _interface->logger->log(F("Sending initial information... "));

  _fillMqttTopic(PSTR("/$online"));
  if (!_publishRetainedOrFail("true")) return;

  char nodes[HomieNode::getNodeCount() * (MAX_NODE_ID_LENGTH + 1 + MAX_NODE_ID_LENGTH + 1) - 1];
  char *begin = nodes;
  char *ptr = nodes;
  HomieNode::forEach([begin, &ptr](HomieNode* n) {
    if (ptr != begin) *ptr++ = ',';
    auto len = strlen(n->getId());
    memcpy(ptr, n->getId(), len);
    ptr += len;
    *ptr++ = ':';
    len = strlen(n->getType());
    memcpy(ptr, n->getType(), len);
    ptr += len;
  });
  *ptr = '\0';
  _fillMqttTopic(PSTR("/$nodes"));
  if (!_publishRetainedOrFail(nodes)) return;

  _fillMqttTopic(PSTR("/$name"));
  if (!_publishRetainedOrFail(_interface->config->get().name)) return;

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
  _fillMqttTopic(PSTR("/$localip"));
  if (!_publishRetainedOrFail(localIpStr)) return;

  _fillMqttTopic(PSTR("/$fwname"));
  if (!_publishRetainedOrFail(_interface->firmware.name)) return;

  _fillMqttTopic(PSTR("/$fwversion"));
  if (!_publishRetainedOrFail(_interface->firmware.version)) return;

  _interface->logger->logln(F(" OK"));

  _fillMqttTopic(PSTR("/+/+/set"));
  _interface->logger->log(F("Subscribing to topics... "));
  if (!_subscribe1OrFail()) return;

  _fillMqttTopic(PSTR("/$reset"));
  if (!_subscribe1OrFail()) return;

  if (_interface->config->get().ota.enabled) {
    _fillMqttTopic(PSTR("/$ota"));
    if (!_subscribe1OrFail()) return;
  }

  _interface->logger->logln(F(" OK"));
}

void BootNormal::_mqttCallback(char* topic, char* payload) {
  String message = String(payload);
  String unified = String(topic);
  unified.remove(0, strlen(_interface->config->get().mqtt.baseTopic) + strlen(_interface->config->get().deviceId) + 1); // Remove devices/${id}/ --- +1 for /

  // Device properties
  if (_interface->config->get().ota.enabled && unified == "$ota") {
    if (message != _interface->firmware.version) {
      _interface->logger->log(F("✴ OTA available (version "));
      _interface->logger->log(message);
      _interface->logger->logln(F(")"));
      if (strlen(payload) + 1 <= MAX_FIRMWARE_VERSION_LENGTH) {
        strcpy(_otaVersion, payload);
        _flaggedForOta = true;
        _interface->logger->logln(F("Flagged for OTA"));
      } else {
        _interface->logger->logln(F("Version string received is too long"));
      }
    }
    return;
  } else if (unified == "$reset" && message == "true") {
    _fillMqttTopic(PSTR("/$reset"));
    _interface->mqttClient->publish("false", true);
    _flaggedForReset = true;
    _interface->logger->logln(F("Flagged for reset by network"));
    return;
  }

  // Implicit node properties
  unified.remove(unified.length() - 4, 4); // Remove /set
  uint16_t separator = 0;
  for (uint16_t i = 0; i < unified.length(); i++) {
    if (unified.charAt(i) == '/') {
      separator = i;
      break;
    }
  }
  String node = unified.substring(0, separator);
  String property = unified.substring(separator + 1);
  HomieNode *homieNode = HomieNode::find(node);
  if (homieNode == 0) {
    _interface->logger->log(F("Node "));
    _interface->logger->log(node);
    _interface->logger->logln(F(" not registered"));
    return;
  }

  int homieNodePropertyIndex = -1;
  for (int i = 0; i < homieNode->getSubscriptionsCount(); i++) {
    Subscription subscription = homieNode->getSubscriptions()[i];
    if (property == subscription.property) {
      homieNodePropertyIndex = i;
      break;
    }
  }

  if (!homieNode->getSubscribeToAll() && homieNodePropertyIndex == -1) {
    _interface->logger->log(F("Node "));
    _interface->logger->log(node);
    _interface->logger->log(F(" not subscribed to "));
    _interface->logger->logln(property);
    return;
  }

  _interface->logger->logln(F("Calling global input handler..."));
  bool handled = _interface->globalInputHandler(node, property, message);
  if (handled) return;

  _interface->logger->logln(F("Calling node input handler..."));
  handled = homieNode->handleInput(property, message);
  if (handled) return;

  if (homieNodePropertyIndex != -1) { // might not if subscribed to all only
    Subscription homieNodeSubscription = homieNode->getSubscriptions()[homieNodePropertyIndex];
    _interface->logger->logln(F("Calling property input handler..."));
    handled = homieNodeSubscription.inputHandler(message);
  }

  if (!handled){
    _interface->logger->logln(F("No handlers handled the following packet:"));
    _interface->logger->log(F("  • Node ID: "));
    _interface->logger->logln(node);
    _interface->logger->log(F("  • Property: "));
    _interface->logger->logln(property);
    _interface->logger->log(F("  • Value: "));
    _interface->logger->logln(message);
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

  _wifiGotIpHandler = WiFi.onStationModeGotIP(std::bind(&BootNormal::_onWifiGotIp, this, std::placeholders::_1));
  _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&BootNormal::_onWifiDisconnected, this, std::placeholders::_1));

  _interface->mqttClient->initMqtt(_interface->config->get().mqtt.server.ssl.enabled);

  if (_interface->reset.enabled) {
    pinMode(_interface->reset.triggerPin, INPUT_PULLUP);

    _resetDebouncer.attach(_interface->reset.triggerPin);
    _resetDebouncer.interval(_interface->reset.triggerTime);
  }

  _interface->config->log();

  if (_interface->config->get().mqtt.server.ssl.enabled) {
    _interface->logger->log(F("SSL enabled: pushing CPU frequency to 160MHz... "));
    if (system_update_cpu_freq(SYS_CPU_160MHZ)) {
      _interface->logger->logln(F("OK"));
    } else {
      _interface->logger->logln(F("Failure"));
      _interface->logger->logln(F("Rebooting..."));
      ESP.restart();
    }
  }

  HomieNode::forEach([] (HomieNode* n) { n->setup(); });

  _wifiConnect();
}

void BootNormal::loop() {
  Boot::loop();

  _handleReset();

  if (_flaggedForReset && _interface->reset.able) {
    _interface->logger->logln(F("Device is in a resettable state"));
    _interface->config->erase();
    _interface->logger->logln(F("Configuration erased"));

    _interface->logger->logln(F("Triggering HOMIE_ABOUT_TO_RESET event..."));
    _interface->eventHandler(HOMIE_ABOUT_TO_RESET);

    _interface->logger->logln(F("↻ Rebooting into config mode..."));
    ESP.restart();
  }

  if (_flaggedForOta && _interface->reset.able) {
    _interface->logger->logln(F("Device is in a resettable state"));
    _interface->config->setOtaMode(true, _otaVersion);

    _interface->logger->logln(F("↻ Rebooting into OTA mode..."));
    ESP.restart();
  }

  _interface->readyToOperate = false;

  if (WiFi.status() != WL_CONNECTED) return;

  if (!_interface->mqttClient->connected()) {
    _mqttConnectNotified = false;
    if (!_mqttDisconnectNotified) {
      _mqttReconnectTimer.reset();
      _uptimeTimer.reset();
      _signalQualityTimer.reset();
      _interface->logger->logln(F("✖ MQTT disconnected"));
      _interface->logger->logln(F("Triggering HOMIE_MQTT_DISCONNECTED event..."));
      _interface->eventHandler(HOMIE_MQTT_DISCONNECTED);
      _mqttDisconnectNotified = true;
    }

    if (_mqttReconnectTimer.check()) {
      _interface->logger->logln(F("↕ Attempting to connect to MQTT..."));
      _mqttReconnectTimer.tick();
      if (_interface->led.enabled) _interface->blinker->start(LED_MQTT_DELAY);
      _mqttConnect();
    }
    return;
  } else {
    if (_interface->led.enabled) _interface->blinker->stop();
  }

  _interface->readyToOperate = true;

  _mqttDisconnectNotified = false;
  if (!_mqttConnectNotified) {
    _interface->logger->logln(F("✔ MQTT ready"));
    _interface->logger->logln(F("Triggering HOMIE_MQTT_CONNECTED event..."));
    _interface->eventHandler(HOMIE_MQTT_CONNECTED);
    HomieNode::forEach([] (HomieNode* n) { n->onReadyToOperate(); });
    _mqttConnectNotified = true;
  }

  if (!_setupFunctionCalled) {
    _interface->logger->logln(F("Calling setup function..."));
    _interface->setupFunction();
    _setupFunctionCalled = true;
  }

  if (_signalQualityTimer.check()) {
    int32_t rssi = WiFi.RSSI();
    uint8_t quality;
    if (rssi <= -100) {
      quality = 0;
    } else if (rssi >= -50) {
      quality = 100;
    } else {
      quality = 2 * (rssi + 100);
    }

    char qualityStr[3 + 1];
    itoa(quality, qualityStr, 10);

    _interface->logger->log(F("Sending Wi-Fi signal quality ("));
    _interface->logger->log(qualityStr);
    _interface->logger->log(F("%)... "));

    _fillMqttTopic(PSTR("/$signal"));
    if (_interface->mqttClient->publish(qualityStr, true)) {
      _interface->logger->logln(F(" OK"));
      _signalQualityTimer.tick();
    } else {
      _interface->logger->logln(F(" Failure"));
    }
  }

  if (_uptimeTimer.check()) {
    _uptime.update();

    char uptimeStr[10 + 1];
    itoa(_uptime.getSeconds(), uptimeStr, 10);

    _interface->logger->log(F("Sending uptime ("));
    _interface->logger->log(_uptime.getSeconds());
    _interface->logger->log(F("s)... "));

    _fillMqttTopic(PSTR("/$uptime"));
    if (_interface->mqttClient->publish(uptimeStr, true)) {
      _interface->logger->logln(F(" OK"));
      _uptimeTimer.tick();
    } else {
      _interface->logger->logln(F(" Failure"));
    }
  }

  _interface->loopFunction();

  _interface->mqttClient->loop();

  HomieNode::forEach([] (HomieNode* n) { n->loop(); });
}

#include "BootNormal.hpp"

using namespace HomieInternals;

BootNormal::BootNormal()
: Boot("normal")
, _setupFunctionCalled(false)
, _wifiConnectNotified(false)
, _wifiDisconnectNotified(true)
, _mqttConnectNotified(false)
, _mqttDisconnectNotified(true)
, _otaVersion {'\0'}
, _flaggedForOta(false)
, _flaggedForReset(false)
{
  this->_wifiReconnectTimer.setInterval(WIFI_RECONNECT_INTERVAL);
  this->_mqttReconnectTimer.setInterval(MQTT_RECONNECT_INTERVAL);
  this->_signalQualityTimer.setInterval(SIGNAL_QUALITY_SEND_INTERVAL);
  this->_uptimeTimer.setInterval(UPTIME_SEND_INTERVAL);
}

BootNormal::~BootNormal() {
}

void BootNormal::_fillMqttTopic(PGM_P topic) {
  strcpy(this->_interface->mqttClient->getTopicBuffer(), this->_interface->config->get().mqtt.baseTopic);
  strcat(this->_interface->mqttClient->getTopicBuffer(), this->_interface->config->get().deviceId);
  strcat_P(this->_interface->mqttClient->getTopicBuffer(), topic);
}

bool BootNormal::_publishRetainedOrFail(const char* message) {
  if (!this->_interface->mqttClient->publish(message, true)) {
    this->_interface->mqttClient->disconnect();
    this->_interface->logger->logln(F(" Failed"));
    return false;
  }

  return true;
}

bool BootNormal::_subscribe1OrFail() {
  if (!this->_interface->mqttClient->subscribe(1)) {
    this->_interface->mqttClient->disconnect();
    this->_interface->logger->logln(F(" Failed"));
    return false;
  }

  return true;
}

void BootNormal::_wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(this->_interface->config->get().wifi.ssid, this->_interface->config->get().wifi.password);
}

void BootNormal::_mqttConnect() {
  String host = this->_interface->config->get().mqtt.server.host;
  unsigned int port = this->_interface->config->get().mqtt.server.port;
  if (this->_interface->config->get().mqtt.server.mdns.enabled) {
    this->_interface->logger->log(F("Querying mDNS service "));
    this->_interface->logger->logln(this->_interface->config->get().mqtt.server.mdns.service);
    MdnsQueryResult result = Helpers::mdnsQuery(this->_interface->config->get().mqtt.server.mdns.service);
    if (result.success) {
      host = result.ip.toString();
      port = result.port;
      this->_interface->logger->log(F("✔ "));
      this->_interface->logger->log(F(" service found at "));
      this->_interface->logger->log(host.c_str());
      this->_interface->logger->log(F(":"));
      this->_interface->logger->logln(port);
    } else {
      this->_interface->logger->logln(F("✖ Service not found"));
      return;
    }
  }

  this->_interface->mqttClient->setServer(host.c_str(), port, this->_interface->config->get().mqtt.server.ssl.fingerprint);
  this->_interface->mqttClient->setCallback(std::bind(&BootNormal::_mqttCallback, this, std::placeholders::_1, std::placeholders::_2));

  char clientId[MAX_WIFI_SSID_LENGTH];
  strcpy(clientId, this->_interface->brand);
  strcat_P(clientId, PSTR("-"));
  strcat(clientId, this->_interface->config->get().deviceId);

  this->_fillMqttTopic(PSTR("/$online"));
  if (this->_interface->mqttClient->connect(clientId, "false", 2, true, this->_interface->config->get().mqtt.auth, this->_interface->config->get().mqtt.username, this->_interface->config->get().mqtt.password)) {
    this->_interface->logger->logln(F("Connected"));
    this->_mqttSetup();
  } else {
    this->_interface->logger->log(F("✖ Cannot connect, reason: "));
    switch (this->_interface->mqttClient->getState()) {
      case MQTT_CONNECTION_TIMEOUT:
        this->_interface->logger->logln(F("MQTT_CONNECTION_TIMEOUT"));
        break;
      case MQTT_CONNECTION_LOST:
        this->_interface->logger->logln(F("MQTT_CONNECTION_LOST"));
        break;
      case MQTT_CONNECT_FAILED:
        this->_interface->logger->logln(F("MQTT_CONNECT_FAILED"));
        break;
      case MQTT_DISCONNECTED:
        this->_interface->logger->logln(F("MQTT_DISCONNECTED"));
        break;
      case MQTT_CONNECTED:
        this->_interface->logger->logln(F("MQTT_CONNECTED (?)"));
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        this->_interface->logger->logln(F("MQTT_CONNECT_BAD_PROTOCOL"));
        break;
      case MQTT_CONNECT_BAD_CLIENT_ID:
        this->_interface->logger->logln(F("MQTT_CONNECT_BAD_CLIENT_ID"));
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        this->_interface->logger->logln(F("MQTT_CONNECT_UNAVAILABLE"));
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        this->_interface->logger->logln(F("MQTT_CONNECT_BAD_CREDENTIALS"));
        break;
      case MQTT_CONNECT_UNAUTHORIZED:
        this->_interface->logger->logln(F("MQTT_CONNECT_UNAUTHORIZED"));
        break;
      default:
        this->_interface->logger->logln(F("UNKNOWN"));
    }
  }
}

void BootNormal::_mqttSetup() {
  this->_interface->logger->log(F("Sending initial information... "));

  this->_fillMqttTopic(PSTR("/$online"));
  if (!this->_publishRetainedOrFail("true")) return;

  char nodes[MAX_REGISTERED_NODES_COUNT * (MAX_NODE_ID_LENGTH + 1 + MAX_NODE_ID_LENGTH + 1) - 1];
  strcpy_P(nodes, PSTR(""));
  for (int i = 0; i < this->_interface->registeredNodesCount; i++) {
    const HomieNode* node = this->_interface->registeredNodes[i];
    strcat(nodes, node->getId());
    strcat_P(nodes, PSTR(":"));
    strcat(nodes, node->getType());
    if (i != this->_interface->registeredNodesCount - 1) strcat_P(nodes, PSTR(","));
  }
  this->_fillMqttTopic(PSTR("/$nodes"));
  if (!this->_publishRetainedOrFail(nodes)) return;

  this->_fillMqttTopic(PSTR("/$name"));
  if (!this->_publishRetainedOrFail(this->_interface->config->get().name)) return;

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
  this->_fillMqttTopic(PSTR("/$localip"));
  if (!this->_publishRetainedOrFail(localIpStr)) return;

  this->_fillMqttTopic(PSTR("/$fwname"));
  if (!this->_publishRetainedOrFail(this->_interface->firmware.name)) return;

  this->_fillMqttTopic(PSTR("/$fwversion"));
  if (!this->_publishRetainedOrFail(this->_interface->firmware.version)) return;

  this->_interface->logger->logln(F(" OK"));

  this->_fillMqttTopic(PSTR("/+/+/set"));
  this->_interface->logger->log(F("Subscribing to topics... "));
  if (!this->_subscribe1OrFail()) return;

  this->_fillMqttTopic(PSTR("/$reset"));
  if (!this->_subscribe1OrFail()) return;

  if (this->_interface->config->get().ota.enabled) {
    this->_fillMqttTopic(PSTR("/$ota"));
    if (!this->_subscribe1OrFail()) return;
  }

  this->_interface->logger->logln(F(" OK"));
}

void BootNormal::_mqttCallback(char* topic, char* payload) {
  String message = String(payload);
  String unified = String(topic);
  unified.remove(0, strlen(this->_interface->config->get().mqtt.baseTopic) + strlen(this->_interface->config->get().deviceId) + 1); // Remove devices/${id}/ --- +1 for /

  // Device properties
  if (this->_interface->config->get().ota.enabled && unified == "$ota") {
    if (message != this->_interface->firmware.version) {
      this->_interface->logger->log(F("✴ OTA available (version "));
      this->_interface->logger->log(message);
      this->_interface->logger->logln(F(")"));
      if (strlen(payload) + 1 <= MAX_FIRMWARE_VERSION_LENGTH) {
        strcpy(this->_otaVersion, payload);
        this->_flaggedForOta = true;
        this->_interface->logger->logln(F("Flagged for OTA"));
      } else {
        this->_interface->logger->logln(F("Version string received is too long"));
      }
    }
    return;
  } else if (unified == "$reset" && message == "true") {
    this->_fillMqttTopic(PSTR("/$reset"));
    this->_interface->mqttClient->publish("false", true);
    this->_flaggedForReset = true;
    this->_interface->logger->logln(F("Flagged for reset by network"));
    return;
  }

  // Implicit node properties
  unified.remove(unified.length() - 4, 4); // Remove /set
  unsigned int separator = 0;
  for (unsigned int i = 0; i < unified.length(); i++) {
    if (unified.charAt(i) == '/') {
      separator = i;
      break;
    }
  }
  String node = unified.substring(0, separator);
  String property = unified.substring(separator + 1);

  int homieNodeIndex = -1;
  for (int i = 0; i < this->_interface->registeredNodesCount; i++) {
    const HomieNode* homieNode = this->_interface->registeredNodes[i];
    if (node == homieNode->getId()) {
      homieNodeIndex = i;
      break;
    }
  }

  if (homieNodeIndex == -1) {
    this->_interface->logger->log(F("Node "));
    this->_interface->logger->log(node);
    this->_interface->logger->logln(F(" not registered"));
    return;
  }

  const HomieNode* homieNode = this->_interface->registeredNodes[homieNodeIndex];

  int homieNodePropertyIndex = -1;
  for (int i = 0; i < homieNode->getSubscriptionsCount(); i++) {
    Subscription subscription = homieNode->getSubscriptions()[i];
    if (property == subscription.property) {
      homieNodePropertyIndex = i;
      break;
    }
  }

  if (!homieNode->getSubscribeToAll() && homieNodePropertyIndex == -1) {
    this->_interface->logger->log(F("Node "));
    this->_interface->logger->log(node);
    this->_interface->logger->log(F(" not subscribed to "));
    this->_interface->logger->logln(property);
    return;
  }

  this->_interface->logger->logln(F("Calling global input handler..."));
  bool handled = this->_interface->globalInputHandler(node, property, message);
  if (handled) return;

  this->_interface->logger->logln(F("Calling node input handler..."));
  handled = homieNode->getInputHandler()(property, message);
  if (handled) return;

  if (homieNodePropertyIndex != -1) { // might not if subscribed to all only
    Subscription homieNodeSubscription = homieNode->getSubscriptions()[homieNodePropertyIndex];
    this->_interface->logger->logln(F("Calling property input handler..."));
    handled = homieNodeSubscription.inputHandler(message);
  }

  if (!handled){
    this->_interface->logger->logln(F("No handlers handled the following packet:"));
    this->_interface->logger->log(F("  • Node ID: "));
    this->_interface->logger->logln(node);
    this->_interface->logger->log(F("  • Property: "));
    this->_interface->logger->logln(property);
    this->_interface->logger->log(F("  • Value: "));
    this->_interface->logger->logln(message);
  }
}

void BootNormal::_handleReset() {
  if (this->_interface->reset.enabled) {
    this->_resetDebouncer.update();

    if (this->_resetDebouncer.read() == this->_interface->reset.triggerState) {
      this->_flaggedForReset = true;
      this->_interface->logger->logln(F("Flagged for reset by pin"));
    }
  }

  if (this->_interface->reset.userFunction()) {
    this->_flaggedForReset = true;
    this->_interface->logger->logln(F("Flagged for reset by function"));
  }
}

void BootNormal::setup() {
  Boot::setup();

  this->_interface->mqttClient->initMqtt(this->_interface->config->get().mqtt.server.ssl.enabled);

  if (this->_interface->reset.enabled) {
    pinMode(this->_interface->reset.triggerPin, INPUT_PULLUP);

    this->_resetDebouncer.attach(this->_interface->reset.triggerPin);
    this->_resetDebouncer.interval(this->_interface->reset.triggerTime);
  }

  this->_interface->config->log();

  if (this->_interface->config->get().mqtt.server.ssl.enabled) {
    this->_interface->logger->log(F("SSL enabled: pushing CPU frequency to 160MHz... "));
    if (system_update_cpu_freq(SYS_CPU_160MHZ)) {
      this->_interface->logger->logln(F("OK"));
    } else {
      this->_interface->logger->logln(F("Failure"));
      this->_interface->logger->logln(F("Rebooting..."));
      ESP.restart();
    }
  }
}

void BootNormal::loop() {
  Boot::loop();

  this->_handleReset();

  if (this->_flaggedForReset && this->_interface->reset.able) {
    this->_interface->logger->logln(F("Device is in a resettable state"));
    this->_interface->config->erase();
    this->_interface->logger->logln(F("Configuration erased"));

    this->_interface->logger->logln(F("Triggering HOMIE_ABOUT_TO_RESET event..."));
    this->_interface->eventHandler(HOMIE_ABOUT_TO_RESET);

    this->_interface->logger->logln(F("↻ Rebooting into config mode..."));
    ESP.restart();
  }

  if (this->_flaggedForOta && this->_interface->reset.able) {
    this->_interface->logger->logln(F("Device is in a resettable state"));
    this->_interface->config->setOtaMode(true, this->_otaVersion);

    this->_interface->logger->logln(F("↻ Rebooting into OTA mode..."));
    ESP.restart();
  }

  this->_interface->readyToOperate = false;

  if (WiFi.status() != WL_CONNECTED) {
    this->_wifiConnectNotified = false;
    if (!this->_wifiDisconnectNotified) {
      this->_wifiReconnectTimer.reset();
      this->_uptimeTimer.reset();
      this->_signalQualityTimer.reset();
      this->_interface->logger->logln(F("✖ Wi-Fi disconnected"));
      this->_interface->logger->logln(F("Triggering HOMIE_WIFI_DISCONNECTED event..."));
      this->_interface->eventHandler(HOMIE_WIFI_DISCONNECTED);
      this->_wifiDisconnectNotified = true;
    }

    if (this->_wifiReconnectTimer.check()) {
      this->_interface->logger->logln(F("↕ Attempting to connect to Wi-Fi..."));
      this->_wifiReconnectTimer.tick();
      if (this->_interface->led.enabled) {
        this->_interface->blinker->start(LED_WIFI_DELAY);
      }
      this->_wifiConnect();
    }
    return;
  }

  this->_wifiDisconnectNotified = false;
  if (!this->_wifiConnectNotified) {
    this->_interface->logger->logln(F("✔ Wi-Fi connected"));
    this->_interface->logger->logln(F("Triggering HOMIE_WIFI_CONNECTED event..."));
    this->_interface->eventHandler(HOMIE_WIFI_CONNECTED);
    this->_wifiConnectNotified = true;
    MDNS.begin(this->_interface->config->get().deviceId);
  }

  if (!this->_interface->mqttClient->connected()) {
    this->_mqttConnectNotified = false;
    if (!this->_mqttDisconnectNotified) {
      this->_mqttReconnectTimer.reset();
      this->_uptimeTimer.reset();
      this->_signalQualityTimer.reset();
      this->_interface->logger->logln(F("✖ MQTT disconnected"));
      this->_interface->logger->logln(F("Triggering HOMIE_MQTT_DISCONNECTED event..."));
      this->_interface->eventHandler(HOMIE_MQTT_DISCONNECTED);
      this->_mqttDisconnectNotified = true;
    }

    if (this->_mqttReconnectTimer.check()) {
      this->_interface->logger->logln(F("↕ Attempting to connect to MQTT..."));
      this->_mqttReconnectTimer.tick();
      if (this->_interface->led.enabled) {
        this->_interface->blinker->start(LED_MQTT_DELAY);
      }
      this->_mqttConnect();
    }
    return;
  } else {
    if (this->_interface->led.enabled) {
      this->_interface->blinker->stop();
    }
  }

  this->_interface->readyToOperate = true;

  this->_mqttDisconnectNotified = false;
  if (!this->_mqttConnectNotified) {
    this->_interface->logger->logln(F("✔ MQTT ready"));
    this->_interface->logger->logln(F("Triggering HOMIE_MQTT_CONNECTED event..."));
    this->_interface->eventHandler(HOMIE_MQTT_CONNECTED);
    this->_mqttConnectNotified = true;
  }

  if (!this->_setupFunctionCalled) {
    this->_interface->logger->logln(F("Calling setup function..."));
    this->_interface->setupFunction();
    this->_setupFunctionCalled = true;
  }

  if (this->_signalQualityTimer.check()) {
    int32_t rssi = WiFi.RSSI();
    unsigned char quality;
    if (rssi <= -100) {
      quality = 0;
    } else if (rssi >= -50) {
      quality = 100;
    } else {
      quality = 2 * (rssi + 100);
    }

    char qualityStr[3 + 1];
    itoa(quality, qualityStr, 10);

    this->_interface->logger->log(F("Sending Wi-Fi signal quality ("));
    this->_interface->logger->log(qualityStr);
    this->_interface->logger->log(F("%)... "));

    this->_fillMqttTopic(PSTR("/$signal"));
    if (this->_interface->mqttClient->publish(qualityStr, true)) {
      this->_interface->logger->logln(F(" OK"));
      this->_signalQualityTimer.tick();
    } else {
      this->_interface->logger->logln(F(" Failure"));
    }
  }

  if (this->_uptimeTimer.check()) {
    this->_uptime.update();

    char uptimeStr[10 + 1];
    itoa(this->_uptime.getSeconds(), uptimeStr, 10);

    this->_interface->logger->log(F("Sending uptime ("));
    this->_interface->logger->log(this->_uptime.getSeconds());
    this->_interface->logger->log(F("s)... "));

    this->_fillMqttTopic(PSTR("/$uptime"));
    if (this->_interface->mqttClient->publish(uptimeStr, true)) {
      this->_interface->logger->logln(F(" OK"));
      this->_uptimeTimer.tick();
    } else {
      this->_interface->logger->logln(F(" Failure"));
    }
  }

  this->_interface->loopFunction();

  this->_interface->mqttClient->loop();
}

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
  strcpy(MqttClient.getTopicBuffer(), Config.get().mqtt.baseTopic);
  strcat(MqttClient.getTopicBuffer(), Config.get().deviceId);
  strcat_P(MqttClient.getTopicBuffer(), topic);
}

bool BootNormal::_publishRetainedOrFail(const char* message) {
  if (!MqttClient.publish(message, true)) {
    MqttClient.disconnect();
    Logger.logln(F(" Failed"));
    return false;
  }

  return true;
}

bool BootNormal::_subscribe1OrFail() {
  if (!MqttClient.subscribe(1)) {
    MqttClient.disconnect();
    Logger.logln(F(" Failed"));
    return false;
  }

  return true;
}

void BootNormal::_wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(Config.get().wifi.ssid, Config.get().wifi.password);
}

void BootNormal::_mqttConnect() {
  const char* host = Config.get().mqtt.server.host;
  unsigned int port = Config.get().mqtt.server.port;
  /* if (Config.get().mqtt.mdns) {
    Logger.log(F("Querying mDNS service "));
    Logger.logln(Config.get().mqtt.mdnsService);

    int n = MDNS.queryService(Config.get().mqtt.mdnsService, "tcp");
    if (n == 0) {
      Logger.logln(F("No services found"));
      return;
    } else {
      Logger.log(F("✔ "));
      Logger.log(n);
      Logger.logln(F(" service(s) found, using first"));
      host = MDNS.IP(0);
      port = MDNS.port(0);
    }
  } */

  MqttClient.setServer(host, port, Config.get().mqtt.server.ssl.fingerprint);
  MqttClient.setCallback(std::bind(&BootNormal::_mqttCallback, this, std::placeholders::_1, std::placeholders::_2));

  char clientId[MAX_WIFI_SSID_LENGTH];
  strcpy(clientId, this->_interface->brand);
  strcat_P(clientId, PSTR("-"));
  strcat(clientId, Config.get().deviceId);

  this->_fillMqttTopic(PSTR("/$online"));
  if (MqttClient.connect(clientId, "false", 2, true, Config.get().mqtt.auth, Config.get().mqtt.username, Config.get().mqtt.password)) {
    Logger.logln(F("Connected"));
    this->_mqttSetup();
  } else {
    Logger.log(F("✖ Cannot connect, reason: "));
    switch (MqttClient.getState()) {
      case MQTT_CONNECTION_TIMEOUT:
        Logger.logln(F("MQTT_CONNECTION_TIMEOUT"));
        break;
      case MQTT_CONNECTION_LOST:
        Logger.logln(F("MQTT_CONNECTION_LOST"));
        break;
      case MQTT_CONNECT_FAILED:
        Logger.logln(F("MQTT_CONNECT_FAILED"));
        break;
      case MQTT_DISCONNECTED:
        Logger.logln(F("MQTT_DISCONNECTED"));
        break;
      case MQTT_CONNECTED:
        Logger.logln(F("MQTT_CONNECTED (?)"));
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        Logger.logln(F("MQTT_CONNECT_BAD_PROTOCOL"));
        break;
      case MQTT_CONNECT_BAD_CLIENT_ID:
        Logger.logln(F("MQTT_CONNECT_BAD_CLIENT_ID"));
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        Logger.logln(F("MQTT_CONNECT_UNAVAILABLE"));
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        Logger.logln(F("MQTT_CONNECT_BAD_CREDENTIALS"));
        break;
      case MQTT_CONNECT_UNAUTHORIZED:
        Logger.logln(F("MQTT_CONNECT_UNAUTHORIZED"));
        break;
      default:
        Logger.logln(F("UNKNOWN"));
    }
  }
}

void BootNormal::_mqttSetup() {
  Logger.log(F("Sending initial information... "));

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
  if (!this->_publishRetainedOrFail(Config.get().name)) return;

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

  Logger.logln(F(" OK"));

  this->_fillMqttTopic(PSTR("/+/+/set"));
  Logger.log(F("Subscribing to topics... "));
  if (!this->_subscribe1OrFail()) return;

  this->_fillMqttTopic(PSTR("/$reset"));
  if (!this->_subscribe1OrFail()) return;

  if (Config.get().ota.enabled) {
    this->_fillMqttTopic(PSTR("/$ota"));
    if (!this->_subscribe1OrFail()) return;
  }

  Logger.logln(F(" OK"));
}

void BootNormal::_mqttCallback(char* topic, char* payload) {
  String message = String(payload);
  String unified = String(topic);
  unified.remove(0, strlen(Config.get().mqtt.baseTopic) + strlen(Config.get().deviceId) + 1); // Remove devices/${id}/ --- +1 for /

  // Device properties
  if (Config.get().ota.enabled && unified == "$ota") {
    if (message != this->_interface->firmware.version) {
      Logger.log(F("✴ OTA available (version "));
      Logger.log(message);
      Logger.logln(F(")"));
      if (strlen(payload) + 1 <= MAX_FIRMWARE_VERSION_LENGTH) {
        strcpy(this->_otaVersion, payload);
        this->_flaggedForOta = true;
        Logger.logln(F("Flagged for OTA"));
      } else {
        Logger.logln(F("Version string received is too long"));
      }
    }
    return;
  } else if (unified == "$reset" && message == "true") {
    this->_fillMqttTopic(PSTR("/$reset"));
    MqttClient.publish("false", true);
    this->_flaggedForReset = true;
    Logger.logln(F("Flagged for reset by network"));
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
    Logger.log(F("Node "));
    Logger.log(node);
    Logger.logln(F(" not registered"));
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
    Logger.log(F("Node "));
    Logger.log(node);
    Logger.log(F(" not subscribed to "));
    Logger.logln(property);
    return;
  }

  Logger.logln(F("Calling global input handler..."));
  bool handled = this->_interface->globalInputHandler(node, property, message);
  if (handled) return;

  Logger.logln(F("Calling node input handler..."));
  handled = homieNode->getInputHandler()(property, message);
  if (handled) return;

  if (homieNodePropertyIndex != -1) { // might not if subscribed to all only
    Subscription homieNodeSubscription = homieNode->getSubscriptions()[homieNodePropertyIndex];
    Logger.logln(F("Calling property input handler..."));
    handled = homieNodeSubscription.inputHandler(message);
  }

  if (!handled){
    Logger.logln(F("No handlers handled the following packet:"));
    Logger.log(F("  • Node ID: "));
    Logger.logln(node);
    Logger.log(F("  • Property: "));
    Logger.logln(property);
    Logger.log(F("  • Value: "));
    Logger.logln(message);
  }
}

void BootNormal::_handleReset() {
  if (this->_interface->reset.enabled) {
    this->_resetDebouncer.update();

    if (this->_resetDebouncer.read() == this->_interface->reset.triggerState) {
      this->_flaggedForReset = true;
      Logger.logln(F("Flagged for reset by pin"));
    }
  }

  if (this->_interface->reset.userFunction()) {
    this->_flaggedForReset = true;
    Logger.logln(F("Flagged for reset by function"));
  }
}

void BootNormal::setup() {
  Boot::setup();

  MqttClient.initMqtt(Config.get().mqtt.server.ssl.enabled);

  if (this->_interface->reset.enabled) {
    pinMode(this->_interface->reset.triggerPin, INPUT_PULLUP);

    this->_resetDebouncer.attach(this->_interface->reset.triggerPin);
    this->_resetDebouncer.interval(this->_interface->reset.triggerTime);
  }

  Config.log();

  if (Config.get().mqtt.server.ssl.enabled) {
    Logger.log(F("SSL enabled: pushing CPU frequency to 160MHz... "));
    if (system_update_cpu_freq(SYS_CPU_160MHZ)) {
      Logger.logln(F("OK"));
    } else {
      Logger.logln(F("Failure"));
      Logger.logln(F("Rebooting..."));
      ESP.restart();
    }
  }
}

void BootNormal::loop() {
  Boot::loop();

  this->_handleReset();

  if (this->_flaggedForReset && this->_interface->reset.able) {
    Logger.logln(F("Device is in a resettable state"));
    Config.erase();
    Logger.logln(F("Configuration erased"));

    Logger.logln(F("Triggering HOMIE_ABOUT_TO_RESET event..."));
    this->_interface->eventHandler(HOMIE_ABOUT_TO_RESET);

    Logger.logln(F("↻ Rebooting into config mode..."));
    ESP.restart();
  }

  if (this->_flaggedForOta && this->_interface->reset.able) {
    Logger.logln(F("Device is in a resettable state"));
    Config.setOtaMode(true, this->_otaVersion);

    Logger.logln(F("↻ Rebooting into OTA mode..."));
    ESP.restart();
  }

  this->_interface->readyToOperate = false;

  if (WiFi.status() != WL_CONNECTED) {
    this->_wifiConnectNotified = false;
    if (!this->_wifiDisconnectNotified) {
      this->_wifiReconnectTimer.reset();
      this->_uptimeTimer.reset();
      this->_signalQualityTimer.reset();
      Logger.logln(F("✖ Wi-Fi disconnected"));
      Logger.logln(F("Triggering HOMIE_WIFI_DISCONNECTED event..."));
      this->_interface->eventHandler(HOMIE_WIFI_DISCONNECTED);
      this->_wifiDisconnectNotified = true;
    }

    if (this->_wifiReconnectTimer.check()) {
      Logger.logln(F("↕ Attempting to connect to Wi-Fi..."));
      this->_wifiReconnectTimer.tick();
      if (this->_interface->led.enabled) {
        Blinker.start(LED_WIFI_DELAY);
      }
      this->_wifiConnect();
    }
    return;
  }

  this->_wifiDisconnectNotified = false;
  if (!this->_wifiConnectNotified) {
    Logger.logln(F("✔ Wi-Fi connected"));
    Logger.logln(F("Triggering HOMIE_WIFI_CONNECTED event..."));
    this->_interface->eventHandler(HOMIE_WIFI_CONNECTED);
    this->_wifiConnectNotified = true;
  }

  if (!MqttClient.connected()) {
    this->_mqttConnectNotified = false;
    if (!this->_mqttDisconnectNotified) {
      this->_mqttReconnectTimer.reset();
      this->_uptimeTimer.reset();
      this->_signalQualityTimer.reset();
      Logger.logln(F("✖ MQTT disconnected"));
      Logger.logln(F("Triggering HOMIE_MQTT_DISCONNECTED event..."));
      this->_interface->eventHandler(HOMIE_MQTT_DISCONNECTED);
      this->_mqttDisconnectNotified = true;
    }

    if (this->_mqttReconnectTimer.check()) {
      Logger.logln(F("↕ Attempting to connect to MQTT..."));
      this->_mqttReconnectTimer.tick();
      if (this->_interface->led.enabled) {
        Blinker.start(LED_MQTT_DELAY);
      }
      this->_mqttConnect();
    }
    return;
  } else {
    if (this->_interface->led.enabled) {
      Blinker.stop();
    }
  }

  this->_interface->readyToOperate = true;

  this->_mqttDisconnectNotified = false;
  if (!this->_mqttConnectNotified) {
    Logger.logln(F("✔ MQTT ready"));
    Logger.logln(F("Triggering HOMIE_MQTT_CONNECTED event..."));
    this->_interface->eventHandler(HOMIE_MQTT_CONNECTED);
    this->_mqttConnectNotified = true;
  }

  if (!this->_setupFunctionCalled) {
    Logger.logln(F("Calling setup function..."));
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

    Logger.log(F("Sending Wi-Fi signal quality ("));
    Logger.log(qualityStr);
    Logger.log(F("%)... "));

    this->_fillMqttTopic(PSTR("/$signal"));
    if (MqttClient.publish(qualityStr, true)) {
      Logger.logln(F(" OK"));
      this->_signalQualityTimer.tick();
    } else {
      Logger.logln(F(" Failure"));
    }
  }

  if (this->_uptimeTimer.check()) {
    Uptime.update();

    char uptimeStr[10 + 1];
    itoa(Uptime.getSeconds(), uptimeStr, 10);

    Logger.log(F("Sending uptime ("));
    Logger.log(Uptime.getSeconds());
    Logger.log(F("s)... "));

    this->_fillMqttTopic(PSTR("/$uptime"));
    if (MqttClient.publish(uptimeStr, true)) {
      Logger.logln(F(" OK"));
      this->_uptimeTimer.tick();
    } else {
      Logger.logln(F(" Failure"));
    }
  }

  this->_interface->loopFunction();

  MqttClient.loop();
}

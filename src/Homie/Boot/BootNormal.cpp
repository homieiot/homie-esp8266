#include "BootNormal.hpp"

using namespace HomieInternals;

BootNormal::BootNormal(Interface* interface)
: Boot(interface, "normal")
, _interface(interface)
, _lastWifiReconnectAttempt(0)
, _lastMqttReconnectAttempt(0)
, _lastSignalSent(0)
, _setupFunctionCalled(false)
, _wifiConnectNotified(false)
, _wifiDisconnectNotified(true)
, _mqttConnectNotified(false)
, _mqttDisconnectNotified(true)
, _flaggedForOta(false)
, _flaggedForReset(false)
{
  if (Config.get().mqtt.server.ssl.enabled) {
    this->_interface->mqtt = new PubSubClient(this->_wifiClientSecure);
  } else {
    this->_interface->mqtt = new PubSubClient(this->_wifiClient);
  }

  this->_mqttDeviceTopic = new char[strlen(Config.get().mqtt.baseTopic) + strlen(Helpers.getDeviceId()) + 1]();
  strcpy(this->_mqttDeviceTopic, Config.get().mqtt.baseTopic);
  strcat(this->_mqttDeviceTopic, Helpers.getDeviceId());

  this->_mqttTopicBuffer = new char[strlen(this->_mqttDeviceTopic) + 11 + 1](); // Greater topic is /$fwversion (11)
}

BootNormal::~BootNormal() {
  delete this->_interface->mqtt;
  delete[] this->_mqttDeviceTopic;
  delete[] this->_mqttTopicBuffer;
}

void BootNormal::_wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(Config.get().wifi.ssid, Config.get().wifi.password);
}

void BootNormal::_mqttConnect() {
  const char* host = Config.get().mqtt.server.host;
  uint16_t port = Config.get().mqtt.server.port;
  /* if (Config.get().mqtt.mdns) {
    Logger.log("Querying mDNS service ");
    Logger.logln(Config.get().mqtt.mdnsService);

    int n = MDNS.queryService(Config.get().mqtt.mdnsService, "tcp");
    if (n == 0) {
      Logger.logln("No services found");
      return;
    } else {
      Logger.log(String(n));
      Logger.logln(" service(s) found, using first");
      host = MDNS.IP(0);
      port = MDNS.port(0);
    }
  } */

  this->_interface->mqtt->setServer(host, port);
  this->_interface->mqtt->setCallback(std::bind(&BootNormal::_mqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$online"));

  char client_id[MAX_LENGTH_WIFI_SSID] = "";
  strcat(client_id, this->_interface->brand);
  strcat_P(client_id, PSTR("-"));
  strcat(client_id, Helpers.getDeviceId());

  bool connectResult;
  if (Config.get().mqtt.auth) {
    connectResult = this->_interface->mqtt->connect(client_id, Config.get().mqtt.username, Config.get().mqtt.password, this->_mqttTopicBuffer, 2, true, "false");
  } else {
    connectResult = this->_interface->mqtt->connect(client_id, this->_mqttTopicBuffer, 2, true, "false");
  }

  if (connectResult) {
    if (Config.get().mqtt.server.ssl.enabled && !(strcmp_P(Config.get().mqtt.server.ssl.fingerprint, PSTR("")) == 0)) {
      Logger.logln(F("Checking certificate"));
      if(!this->_wifiClientSecure.verify(Config.get().mqtt.server.ssl.fingerprint, Config.get().mqtt.server.host)) {
        Logger.logln(F("✖ MQTT SSL certificate mismatch"));
        this->_interface->mqtt->disconnect();
        return;
      }
    }

    this->_mqttSetup();
  }
}

void BootNormal::_mqttSetup() {
  Logger.logln(F("Sending initial informations"));

  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$nodes"));

  int nodesLength = 0;
  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode node = this->_interface->registeredNodes[i];
    nodesLength += strlen(node.id);
    nodesLength += 1; // :
    nodesLength += strlen(node.type);
    nodesLength += 1; // ,
  }
  nodesLength -= 1; // Last ,
  nodesLength += 1; // Leading \0

  std::unique_ptr<char[]> nodes(new char[nodesLength]);
  strcpy_P(nodes.get(), PSTR(""));
  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode node = this->_interface->registeredNodes[i];
    strcat(nodes.get(), node.id);
    strcat_P(nodes.get(), PSTR(":"));
    strcat(nodes.get(), node.type);
    if (i != this->_interface->registeredNodes.size() - 1) strcat_P(nodes.get(), PSTR(","));
  }
  this->_interface->mqtt->publish(this->_mqttTopicBuffer, nodes.get(), true);

  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$online"));
  this->_interface->mqtt->publish(this->_mqttTopicBuffer, "true", true);

  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$name"));
  this->_interface->mqtt->publish(this->_mqttTopicBuffer, Config.get().name, true);

  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$localip"));
  IPAddress local_ip = WiFi.localIP();
  char local_ip_str[15 + 1];
  char local_ip_part_str[3 + 1];
  itoa(local_ip[0], local_ip_part_str, 10);
  strcpy(local_ip_str, local_ip_part_str);
  strcat_P(local_ip_str, PSTR("."));
  itoa(local_ip[1], local_ip_part_str, 10);
  strcat(local_ip_str, local_ip_part_str);
  strcat_P(local_ip_str, PSTR("."));
  itoa(local_ip[2], local_ip_part_str, 10);
  strcat(local_ip_str, local_ip_part_str);
  strcat_P(local_ip_str, PSTR("."));
  itoa(local_ip[3], local_ip_part_str, 10);
  strcat(local_ip_str, local_ip_part_str);
  this->_interface->mqtt->publish(this->_mqttTopicBuffer, local_ip_str, true);

  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$fwname"));
  this->_interface->mqtt->publish(this->_mqttTopicBuffer, this->_interface->firmware.name, true);

  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$fwversion"));
  this->_interface->mqtt->publish(this->_mqttTopicBuffer, this->_interface->firmware.version, true);

  strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
  strcat_P(this->_mqttTopicBuffer, PSTR("/$reset"));
  this->_interface->mqtt->subscribe(this->_mqttTopicBuffer, 1);

  if (Config.get().ota.enabled) {
    strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
    strcat_P(this->_mqttTopicBuffer, PSTR("/$ota"));
    this->_interface->mqtt->subscribe(this->_mqttTopicBuffer, 1);
  }

  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode node = this->_interface->registeredNodes[i];
    for (int i = 0; i < node.subscriptions.size(); i++) {
      Subscription subscription = node.subscriptions[i];

      std::unique_ptr<char[]> topicSub(new char[strlen(this->_mqttDeviceTopic) + 1 + strlen(node.id) + 1 + strlen(subscription.property) + 4 + 1]);
      strcpy(topicSub.get(), this->_mqttDeviceTopic);
      strcat_P(topicSub.get(), PSTR("/"));
      strcat(topicSub.get(), node.id);
      strcat_P(topicSub.get(), PSTR("/"));
      strcat(topicSub.get(), subscription.property);
      strcat_P(topicSub.get(), PSTR("/set"));
      this->_interface->mqtt->subscribe(topicSub.get(), 1);
      this->_interface->mqtt->loop(); // see knolleary/pubsublient#98
    }
  }
}

void BootNormal::_mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  message.reserve(length);
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  String unified = String(topic);
  unified.remove(0, strlen(this->_mqttDeviceTopic) + 1); // Remove devices/${id}/ --- +1 for /
  if (Config.get().ota.enabled && unified == "$ota") {
    if (message != this->_interface->firmware.version) {
      Logger.log(F("✴ OTA available (version "));
      Logger.log(message);
      Logger.logln(")");
      this->_flaggedForOta = true;
      Serial.println(F("Flagged for OTA"));
    }
    return;
  } else if (unified == "$reset" && message == "true") {
    this->_interface->mqtt->publish(topic, "false", true);
    this->_flaggedForReset = true;
    Logger.logln(F("Flagged for reset by network"));
    return;
  }
  unified.remove(unified.length() - 4, 4); // Remove /set
  int separator;
  for (int i = 0; i < unified.length(); i++) {
    if (unified.charAt(i) == '/') {
      separator = i;
    }
  }
  String node = unified.substring(0, separator);
  String property = unified.substring(separator + 1);

  bool handled = this->_interface->inputHandler(node, property, message);
  if (handled) { return; }

  int homieNodeIndex;
  bool homieNodeFound = false;
  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode homieNode = this->_interface->registeredNodes[i];
    if (node == homieNode.id) {
      homieNodeFound = true;
      homieNodeIndex = i;
      handled = homieNode.inputHandler(property, message);
      break;
    }
  }

  if (!homieNodeFound) { return; }
  if (handled) { return; }

  HomieNode homieNode = this->_interface->registeredNodes[homieNodeIndex];

  for (int i = 0; i < homieNode.subscriptions.size(); i++) {
    Subscription subscription = homieNode.subscriptions[i];
    if (property == subscription.property) {
      handled = subscription.inputHandler(message);
      break;
    }
  }

  if (!handled) {
    Logger.logln(F("No handlers handled the following message:"));
    Logger.log(F("  • Node ID: "));
    Logger.logln(node);
    Logger.log(F("  • Property: "));
    Logger.logln(property);
    Logger.log(F("  • Message: "));
    Logger.logln(message);
  }
}

void BootNormal::_handleReset() {
  if (this->_interface->reset.enable) {
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

  if (this->_interface->reset.enable) {
    pinMode(this->_interface->reset.triggerPin, INPUT_PULLUP);

    this->_resetDebouncer.attach(this->_interface->reset.triggerPin);
    this->_resetDebouncer.interval(this->_interface->reset.triggerTime);
  }

  Config.log();
}

void BootNormal::loop() {
  Boot::loop();

  this->_handleReset();

  if (this->_flaggedForReset && this->_interface->reset.able) {
    Logger.logln(F("Device is in a resettable state"));
    Config.erase();
    Logger.logln(F("Configuration erased"));

    this->_interface->eventHandler(HOMIE_ABOUT_TO_RESET);

    Logger.logln(F("↻ Rebooting in config mode"));
    ESP.restart();
  }

  if (this->_flaggedForOta && this->_interface->reset.able) {
    Logger.logln(F("Device is in a resettable state"));
    Config.setOtaMode(true);

    Logger.logln(F("↻ Rebooting in OTA mode"));
    ESP.restart();
  }

  this->_interface->readyToOperate = false;

  if (WiFi.status() != WL_CONNECTED) {
    this->_wifiConnectNotified = false;
    if (!this->_wifiDisconnectNotified) {
      this->_lastWifiReconnectAttempt = 0;
      Logger.logln(F("✖ Wi-Fi disconnected"));
      this->_interface->eventHandler(HOMIE_WIFI_DISCONNECTED);
      this->_wifiDisconnectNotified = true;
    }

    unsigned long now = millis();
    if (now - this->_lastWifiReconnectAttempt >= WIFI_RECONNECT_INTERVAL || this->_lastWifiReconnectAttempt == 0) {
      Logger.logln(F("⌔ Attempting to connect to Wi-Fi"));
      this->_lastWifiReconnectAttempt = now;
      if (this->_interface->led.enable) {
        Blinker.start(LED_WIFI_DELAY);
      }
      this->_wifiConnect();
    }
    return;
  }

  this->_wifiDisconnectNotified = false;
  if (!this->_wifiConnectNotified) {
    Logger.logln(F("✔ Wi-Fi connected"));
    this->_interface->eventHandler(HOMIE_WIFI_CONNECTED);
    this->_wifiConnectNotified = true;
  }

  if (!this->_interface->mqtt->connected()) {
    this->_mqttConnectNotified = false;
    if (!this->_mqttDisconnectNotified) {
      this->_lastMqttReconnectAttempt = 0;
      Logger.logln(F("✖ MQTT disconnected"));
      this->_interface->eventHandler(HOMIE_MQTT_DISCONNECTED);
      this->_mqttDisconnectNotified = true;
    }

    unsigned long now = millis();
    if (now - this->_lastMqttReconnectAttempt >= MQTT_RECONNECT_INTERVAL || this->_lastMqttReconnectAttempt == 0) {
      Logger.logln(F("⌔ Attempting to connect to MQTT"));
      this->_lastMqttReconnectAttempt = now;
      if (this->_interface->led.enable) {
        Blinker.start(LED_MQTT_DELAY);
      }
      this->_mqttConnect();
    }
    return;
  } else {
    if (this->_interface->led.enable) {
      Blinker.stop();
    }
  }

  this->_interface->readyToOperate = true;

  this->_mqttDisconnectNotified = false;
  if (!this->_mqttConnectNotified) {
    Logger.logln(F("✔ MQTT connected"));
    this->_interface->eventHandler(HOMIE_MQTT_CONNECTED);
    this->_mqttConnectNotified = true;
  }

  if (!this->_setupFunctionCalled) {
    Logger.logln(F("Calling setup function"));
    this->_interface->setupFunction();
    this->_setupFunctionCalled = true;
  }

  unsigned long now = millis();
  if (now - this->_lastSignalSent >= SIGNAL_QUALITY_SEND_INTERVAL || this->_lastSignalSent == 0) {
    Logger.logln(F("Sending Wi-Fi signal quality"));
    int32_t rssi = WiFi.RSSI();
    byte quality;
    if (rssi <= -100) {
      quality = 0;
    } else if (rssi >= -50) {
      quality = 100;
    } else {
      quality = 2 * (rssi + 100);
    }

    char quality_str[3 + 1];
    itoa(quality, quality_str, 10);

    strcpy(this->_mqttTopicBuffer, this->_mqttDeviceTopic);
    strcat(this->_mqttTopicBuffer, "/$signal");

    if (this->_interface->mqtt->publish(this->_mqttTopicBuffer, quality_str, true)) {
      this->_lastSignalSent = now;
    }
  }

  this->_interface->loopFunction();

  this->_interface->mqtt->loop();
}

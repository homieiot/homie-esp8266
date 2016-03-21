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

  strcpy(this->_mqttDeviceTopic, Config.get().mqtt.baseTopic);
  strcat(this->_mqttDeviceTopic, Helpers.getDeviceId());
}

BootNormal::~BootNormal() {
  delete this->_interface->mqtt;
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
  char topic[24 + 1];
  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$online");

  char client_id[MAX_LENGTH_WIFI_SSID] = "";
  strcat(client_id, this->_interface->brand);
  strcat(client_id, "-");
  strcat(client_id, Helpers.getDeviceId());

  bool connectResult;
  if (Config.get().mqtt.auth) {
    connectResult = this->_interface->mqtt->connect(client_id, Config.get().mqtt.username, Config.get().mqtt.password, topic, 2, true, "false");
  } else {
    connectResult = this->_interface->mqtt->connect(client_id, topic, 2, true, "false");
  }

  if (connectResult) {
    if (Config.get().mqtt.server.ssl.enabled && strcmp(Config.get().mqtt.server.ssl.fingerprint, "")) {
      Logger.logln("Checking certificate");
      if(!this->_wifiClientSecure.verify(Config.get().mqtt.server.ssl.fingerprint, Config.get().mqtt.server.host)) {
        Logger.logln("✖ MQTT SSL certificate mismatch");
        this->_interface->mqtt->disconnect();
        return;
      }
    }

    this->_mqttSetup();
  }
}

void BootNormal::_mqttSetup() {
  Logger.logln("Sending initial informations");

  char topic[27 + 1];
  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$nodes");

  int nodesLength = 0;
  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode node = this->_interface->registeredNodes[i];
    nodesLength += strlen(node.id);
    nodesLength += 1; // :
    nodesLength += strlen(node.type);
    nodesLength += 1; // ,
  }
  nodesLength += 1; // Leading \0

  String nodes = String();
  nodes.reserve(nodesLength);
  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode node = this->_interface->registeredNodes[i];
    nodes += node.id;
    nodes += ":";
    nodes += node.type;
    nodes += ",";
  }
  nodes.remove(nodes.length() - 1, 1); // Remove last ,
  this->_interface->mqtt->publish(topic, nodes.c_str(), true);

  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$online");
  this->_interface->mqtt->publish(topic, "true", true);

  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$name");
  this->_interface->mqtt->publish(topic, Config.get().name, true);

  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$localip");
  IPAddress local_ip = WiFi.localIP();
  char local_ip_str[15 + 1];
  char local_ip_part_str[3 + 1];
  itoa(local_ip[0], local_ip_part_str, 10);
  strcpy(local_ip_str, local_ip_part_str);
  strcat(local_ip_str, ".");
  itoa(local_ip[1], local_ip_part_str, 10);
  strcat(local_ip_str, local_ip_part_str);
  strcat(local_ip_str, ".");
  itoa(local_ip[2], local_ip_part_str, 10);
  strcat(local_ip_str, local_ip_part_str);
  strcat(local_ip_str, ".");
  itoa(local_ip[3], local_ip_part_str, 10);
  strcat(local_ip_str, local_ip_part_str);
  this->_interface->mqtt->publish(topic, local_ip_str, true);

  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$fwname");
  this->_interface->mqtt->publish(topic, this->_interface->firmware.name, true);

  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$fwversion");
  this->_interface->mqtt->publish(topic, this->_interface->firmware.version, true);

  strcpy(topic, this->_mqttDeviceTopic);
  strcat(topic, "/$reset");
  this->_interface->mqtt->subscribe(topic, 1);

  if (Config.get().ota.enabled) {
    strcpy(topic, this->_mqttDeviceTopic);
    strcat(topic, "/$ota");
    this->_interface->mqtt->subscribe(topic, 1);
  }

  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode node = this->_interface->registeredNodes[i];
    for (int i = 0; i < node.subscriptions.size(); i++) {
      Subscription subscription = node.subscriptions[i];

      String dynamic_topic;
      dynamic_topic.reserve(strlen(this->_mqttDeviceTopic) + 1 + strlen(node.id) + 1 + strlen(subscription.property) + 4 + 1);
      dynamic_topic = this->_mqttDeviceTopic;
      dynamic_topic += "/";
      dynamic_topic += node.id;
      dynamic_topic += "/";
      dynamic_topic += subscription.property;
      dynamic_topic += "/set";
      this->_interface->mqtt->subscribe(dynamic_topic.c_str(), 1);
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
      Logger.log("✴ OTA available (version ");
      Logger.log(message);
      Logger.logln(")");
      this->_flaggedForOta = true;
      Serial.println("Flagged for OTA");
    }
    return;
  } else if (unified == "$reset" && message == "true") {
    this->_interface->mqtt->publish(topic, "false", true);
    this->_flaggedForReset = true;
    Logger.logln("Flagged for reset by network");
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
    Logger.logln("No handlers handled the following message:");
    Logger.log("  • Node ID: ");
    Logger.logln(node);
    Logger.log("  • Property: ");
    Logger.logln(property);
    Logger.log("  • Message: ");
    Logger.logln(message);
  }
}

void BootNormal::_handleReset() {
  if (this->_interface->reset.enable) {
    this->_resetDebouncer.update();

    if (this->_resetDebouncer.read() == this->_interface->reset.triggerState) {
      this->_flaggedForReset = true;
      Logger.logln("Flagged for reset by pin");
    }
  }

  if (this->_interface->reset.userFunction()) {
    this->_flaggedForReset = true;
    Logger.logln("Flagged for reset by function");
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
    Logger.logln("Device is in a resettable state");
    Config.erase();
    Logger.logln("Configuration erased");

    this->_interface->eventHandler(HOMIE_ABOUT_TO_RESET);

    Logger.logln("↻ Rebooting in config mode");
    ESP.restart();
  }

  if (this->_flaggedForOta && this->_interface->reset.able) {
    Logger.logln("Device is in a resettable state");
    Config.setOtaMode(true);

    Logger.logln("↻ Rebooting in OTA mode");
    ESP.restart();
  }

  this->_interface->readyToOperate = false;

  if (WiFi.status() != WL_CONNECTED) {
    this->_wifiConnectNotified = false;
    if (!this->_wifiDisconnectNotified) {
      this->_lastWifiReconnectAttempt = 0;
      Logger.logln("✖ Wi-Fi disconnected");
      this->_interface->eventHandler(HOMIE_WIFI_DISCONNECTED);
      this->_wifiDisconnectNotified = true;
    }

    unsigned long now = millis();
    if (now - this->_lastWifiReconnectAttempt >= WIFI_RECONNECT_INTERVAL || this->_lastWifiReconnectAttempt == 0) {
      Logger.logln("⌔ Attempting to connect to Wi-Fi");
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
    Logger.logln("✔ Wi-Fi connected");
    this->_interface->eventHandler(HOMIE_WIFI_CONNECTED);
    this->_wifiConnectNotified = true;
  }

  if (!this->_interface->mqtt->connected()) {
    this->_mqttConnectNotified = false;
    if (!this->_mqttDisconnectNotified) {
      this->_lastMqttReconnectAttempt = 0;
      Logger.logln("✖ MQTT disconnected");
      this->_interface->eventHandler(HOMIE_MQTT_DISCONNECTED);
      this->_mqttDisconnectNotified = true;
    }

    unsigned long now = millis();
    if (now - this->_lastMqttReconnectAttempt >= MQTT_RECONNECT_INTERVAL || this->_lastMqttReconnectAttempt == 0) {
      Logger.logln("⌔ Attempting to connect to MQTT");
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
    Logger.logln("✔ MQTT connected");
    this->_interface->eventHandler(HOMIE_MQTT_CONNECTED);
    this->_mqttConnectNotified = true;
  }

  if (!this->_setupFunctionCalled) {
    Logger.logln("Calling setup function");
    this->_interface->setupFunction();
    this->_setupFunctionCalled = true;
  }

  unsigned long now = millis();
  if (now - this->_lastSignalSent >= SIGNAL_QUALITY_SEND_INTERVAL || this->_lastSignalSent == 0) {
    Logger.logln("Sending Wi-Fi signal quality");
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

    char topic[24 + 1];
    strcpy(topic, this->_mqttDeviceTopic);
    strcat(topic, "/$signal");

    if (this->_interface->mqtt->publish(topic, quality_str, true)) {
      this->_lastSignalSent = now;
    }
  }

  this->_interface->loopFunction();

  this->_interface->mqtt->loop();
}

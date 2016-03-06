#include "BootNormal.hpp"

using namespace HomieInternals;

BootNormal::BootNormal(SharedInterface* shared_interface)
: Boot(shared_interface, "normal")
, _shared_interface(shared_interface)
, _last_wifi_reconnect_attempt(0)
, _last_mqtt_reconnect_attempt(0)
, _last_signal_sent(0)
, _wifi_connect_notified(false)
, _wifi_disconnect_notified(false)
, _mqtt_connect_notified(false)
, _mqtt_disconnect_notified(false)
, _flagged_for_ota(false)
, _flagged_for_reset(false)
{
  if (Config.get().mqtt.ssl) {
    this->_shared_interface->mqtt = new PubSubClient(this->_wifiClientSecure);
  } else {
    this->_shared_interface->mqtt = new PubSubClient(this->_wifiClient);
  }

  strcpy(this->_mqtt_base_topic, "devices/");
  strcat(this->_mqtt_base_topic, Helpers.getDeviceId());
}

BootNormal::~BootNormal() {
  delete this->_shared_interface->mqtt;
}

void BootNormal::_wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(Config.get().wifi.ssid, Config.get().wifi.password);
}

void BootNormal::_mqttConnect() {
  this->_shared_interface->mqtt->setServer(Config.get().mqtt.host, Config.get().mqtt.port);
  this->_shared_interface->mqtt->setCallback(std::bind(&BootNormal::_mqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  char topic[24 + 1];
  strcpy(topic, this->_mqtt_base_topic);
  strcat(topic, "/$online");

  char client_id[CONFIG_MAX_LENGTH_WIFI_SSID] = "";
  strcat(client_id, this->_shared_interface->brand);
  strcat(client_id, "-");
  strcat(client_id, Helpers.getDeviceId());

  bool connectResult;
  if (Config.get().mqtt.auth) {
    connectResult = this->_shared_interface->mqtt->connect(client_id, Config.get().mqtt.username, Config.get().mqtt.password, topic, 2, true, "false");
  } else {
    connectResult = this->_shared_interface->mqtt->connect(client_id, topic, 2, true, "false");
  }

  if (connectResult) {
    if (Config.get().mqtt.ssl && !strcmp(Config.get().mqtt.fingerprint, "")) {
      if(!this->_wifiClientSecure.verify(Config.get().mqtt.fingerprint, Config.get().mqtt.host)) {
        Logger.logln("✖ MQTT SSL certificate mismatch");
        this->_shared_interface->mqtt->disconnect();
        return;
      }
    }

    this->_mqttSetup();
  }
}

void BootNormal::_mqttSetup() {
  Logger.logln("Sending initial informations");

  char topic[27 + 1];
  strcpy(topic, this->_mqtt_base_topic);
  strcat(topic, "/$nodes");

  int nodesLength = 0;
  for (int i = 0; i < this->_shared_interface->nodes.size(); i++) {
    HomieNode node = this->_shared_interface->nodes[i];
    nodesLength += strlen(node.id);
    nodesLength += 1; // :
    nodesLength += strlen(node.type);
    nodesLength += 1; // ,
  }
  nodesLength += 1; // Leading \0

  String nodes = String();
  nodes.reserve(nodesLength);
  for (int i = 0; i < this->_shared_interface->nodes.size(); i++) {
    HomieNode node = this->_shared_interface->nodes[i];
    nodes += node.id;
    nodes += ":";
    nodes += node.type;
    nodes += ",";
  }
  nodes.remove(nodes.length() - 1, 1); // Remove last ,
  this->_shared_interface->mqtt->publish(topic, nodes.c_str(), true);

  strcpy(topic, this->_mqtt_base_topic);
  strcat(topic, "/$online");
  this->_shared_interface->mqtt->publish(topic, "true", true);

  strcpy(topic, this->_mqtt_base_topic);
  strcat(topic, "/$name");
  this->_shared_interface->mqtt->publish(topic, Config.get().name, true);

  strcpy(topic, this->_mqtt_base_topic);
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
  this->_shared_interface->mqtt->publish(topic, local_ip_str, true);

  strcpy(topic, this->_mqtt_base_topic);
  strcat(topic, "/$fwname");
  this->_shared_interface->mqtt->publish(topic, this->_shared_interface->fwname, true);

  strcpy(topic, this->_mqtt_base_topic);
  strcat(topic, "/$fwversion");
  this->_shared_interface->mqtt->publish(topic, this->_shared_interface->fwversion, true);

  strcpy(topic, this->_mqtt_base_topic);
  strcat(topic, "/$reset");
  this->_shared_interface->mqtt->subscribe(topic, 1);

  if (Config.get().ota.enabled) {
    strcpy(topic, this->_mqtt_base_topic);
    strcat(topic, "/$ota");
    this->_shared_interface->mqtt->subscribe(topic, 1);
  }

  for (int i = 0; i < this->_shared_interface->nodes.size(); i++) {
    HomieNode node = this->_shared_interface->nodes[i];
    for (int i = 0; i < node.subscriptions.size(); i++) {
      Subscription subscription = node.subscriptions[i];

      String dynamic_topic;
      dynamic_topic.reserve(strlen(this->_mqtt_base_topic) + 1 + strlen(node.id) + 1 + strlen(subscription.property) + 4 + 1);
      dynamic_topic = this->_mqtt_base_topic;
      dynamic_topic += "/";
      dynamic_topic += node.id;
      dynamic_topic += "/";
      dynamic_topic += subscription.property;
      dynamic_topic += "/set";
      this->_shared_interface->mqtt->subscribe(dynamic_topic.c_str(), 1);
      this->_shared_interface->mqtt->loop(); // see knolleary/pubsublient#98
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
  unified.remove(0, strlen(this->_mqtt_base_topic) + 1); // Remove /devices/${id}/ - +1 for /
  if (Config.get().ota.enabled && unified == "$ota") {
    if (message != this->_shared_interface->fwversion) {
      Logger.log("✴ OTA available (version ");
      Logger.log(message);
      Logger.logln(")");
      this->_flagged_for_ota = true;
      Serial.println("Flagged for OTA");
    }
    return;
  } else if (unified == "$reset" && message == "true") {
    this->_shared_interface->mqtt->publish(topic, "false", true);
    this->_flagged_for_reset = true;
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

  bool handled = this->_shared_interface->inputHandler(node, property, message);
  if (handled) { return; }

  int homieNodeIndex;
  bool homieNodeFound = false;
  for (int i = 0; i < this->_shared_interface->nodes.size(); i++) {
    HomieNode homieNode = this->_shared_interface->nodes[i];
    if (node == homieNode.id) {
      homieNodeFound = true;
      homieNodeIndex = i;
      handled = homieNode.inputHandler(property, message);
      break;
    }
  }

  if (!homieNodeFound) { return; }
  if (handled) { return; }

  HomieNode homieNode = this->_shared_interface->nodes[homieNodeIndex];

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
  if (this->_shared_interface->resetTriggerEnabled) {
    this->_resetDebouncer.update();

    if (this->_resetDebouncer.read() == this->_shared_interface->resetTriggerState) {
      this->_flagged_for_reset = true;
      Logger.logln("Flagged for reset by pin");
    }
  }

  if (this->_shared_interface->resetFunction()) {
    this->_flagged_for_reset = true;
    Logger.logln("Flagged for reset by function");
  }
}

void BootNormal::setup() {
  Boot::setup();

  if (this->_shared_interface->resetTriggerEnabled) {
    pinMode(this->_shared_interface->resetTriggerPin, INPUT_PULLUP);

    this->_resetDebouncer.attach(this->_shared_interface->resetTriggerPin);
    this->_resetDebouncer.interval(this->_shared_interface->resetTriggerTime);

    this->_shared_interface->setupFunction();
  }

  Config.log();
}

void BootNormal::loop() {
  Boot::loop();

  this->_handleReset();

  if (this->_flagged_for_reset && this->_shared_interface->resettable) {
    Logger.logln("Device is in a resettable state");
    Config.erase();
    Logger.logln("Configuration erased");

    this->_shared_interface->eventHandler(HOMIE_ABOUT_TO_RESET);

    Logger.logln("↻ Rebooting in config mode");
    ESP.restart();
  }

  if (this->_flagged_for_ota && this->_shared_interface->resettable) {
    Logger.logln("Device is in a resettable state");
    Config.setOtaMode(true);

    Logger.logln("↻ Rebooting in OTA mode");
    ESP.restart();
  }

  this->_shared_interface->readyToOperate = false;

  if (WiFi.status() != WL_CONNECTED) {
    this->_wifi_connect_notified = false;
    if (!this->_wifi_disconnect_notified) {
      this->_last_wifi_reconnect_attempt = 0;
      this->_shared_interface->eventHandler(HOMIE_WIFI_DISCONNECTED);
      this->_wifi_disconnect_notified = true;
    }

    unsigned long now = millis();
    if (now - this->_last_wifi_reconnect_attempt >= WIFI_RECONNECT_INTERVAL || this->_last_wifi_reconnect_attempt == 0) {
      Logger.logln("⌔ Attempting to connect to Wi-Fi");
      this->_last_wifi_reconnect_attempt = now;
      if (this->_shared_interface->useBuiltInLed) {
        Blinker.start(LED_WIFI_DELAY);
      }
      this->_wifiConnect();
    }
    return;
  }

  this->_wifi_disconnect_notified = false;
  if (!this->_wifi_connect_notified) {
    this->_shared_interface->eventHandler(HOMIE_WIFI_CONNECTED);
    this->_wifi_connect_notified = true;
  }

  if (!this->_shared_interface->mqtt->connected()) {
    this->_mqtt_connect_notified = false;
    if (!this->_mqtt_disconnect_notified) {
      this->_last_mqtt_reconnect_attempt = 0;
      this->_shared_interface->eventHandler(HOMIE_MQTT_DISCONNECTED);
      this->_mqtt_disconnect_notified = true;
    }

    unsigned long now = millis();
    if (now - this->_last_mqtt_reconnect_attempt >= MQTT_RECONNECT_INTERVAL || this->_last_mqtt_reconnect_attempt == 0) {
      Logger.logln("⌔ Attempting to connect to MQTT");
      this->_last_mqtt_reconnect_attempt = now;
      if (this->_shared_interface->useBuiltInLed) {
        Blinker.start(LED_MQTT_DELAY);
      }
      this->_mqttConnect();
    }
    return;
  } else {
    if (this->_shared_interface->useBuiltInLed) {
      Blinker.stop();
    }
  }

  this->_mqtt_disconnect_notified = false;
  if (!this->_mqtt_connect_notified) {
    this->_shared_interface->eventHandler(HOMIE_MQTT_CONNECTED);
    this->_mqtt_connect_notified = true;
  }

  unsigned long now = millis();
  if (now - this->_last_signal_sent >= SIGNAL_QUALITY_SEND_INTERVAL || this->_last_signal_sent == 0) {
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
    strcpy(topic, this->_mqtt_base_topic);
    strcat(topic, "/$signal");

    if (this->_shared_interface->mqtt->publish(topic, quality_str, true)) {
      this->_last_signal_sent = now;
    }
  }

  this->_shared_interface->readyToOperate = true;
  this->_shared_interface->loopFunction();

  this->_shared_interface->mqtt->loop();
}

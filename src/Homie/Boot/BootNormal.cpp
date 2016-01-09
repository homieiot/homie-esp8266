#include "BootNormal.hpp"

BootNormal::BootNormal(SharedInterface* shared_interface)
: Boot("normal")
, _shared_interface(shared_interface)
, _last_wifi_reconnect_attempt(0)
, _last_mqtt_reconnect_attempt(0)
, _flagged_for_ota(false)
, _mqtt_base_topic_length(0)
{
  this->_shared_interface->mqtt = new PubSubClient(this->_wifiClient);
}

BootNormal::~BootNormal() {
  delete this->_shared_interface->mqtt;
}

void BootNormal::_wifiConnect() {
  WiFi.hostname(Config.hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(Config.wifi_ssid, Config.wifi_password);
}

void BootNormal::_mqttConnect() {
  this->_shared_interface->mqtt->setServer(Config.homie_host, HOMIE_PORT);
  this->_shared_interface->mqtt->setCallback(std::bind(&BootNormal::_mqttCallback, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  String topic = "devices/";
  topic += Config.hostname;
  topic += "/$online";

  if (this->_shared_interface->mqtt->connect(Config.hostname, topic.c_str(), 2, true, "false")) {
    this->_mqttSetup();
  }
}

void BootNormal::_mqttSetup() {
  String topic = "devices/";
  topic += Config.hostname;
  topic += "/";
  this->_mqtt_base_topic_length = topic.length();
  topic += "$nodes";

  String nodes = String();
  for (int i = 0; i < this->_shared_interface->nodes.size(); i++) {
    Node node = this->_shared_interface->nodes[i];
    nodes += node.name;
    nodes += ":";
    nodes += node.type;
    nodes += ",";
  }
  nodes.remove(nodes.length() - 1, 1); // Remove last ,
  this->_shared_interface->mqtt->publish(topic.c_str(), nodes.c_str(), true);

  topic = "devices/";
  topic += Config.hostname;
  topic += "/$online";
  this->_shared_interface->mqtt->publish(topic.c_str(), "true", true);

  topic = "devices/";
  topic += Config.hostname;
  topic += "/$version";
  this->_shared_interface->mqtt->publish(topic.c_str(), this->_shared_interface->version, true);

  topic = "devices/";
  topic += Config.hostname;
  topic += "/$localip";
  IPAddress local_ip = WiFi.localIP();
  String local_ip_str = String(local_ip[0]);
  local_ip_str += ".";
  local_ip_str += local_ip[1];
  local_ip_str += ".";
  local_ip_str += local_ip[2];
  local_ip_str += ".";
  local_ip_str += local_ip[3];
  this->_shared_interface->mqtt->publish(topic.c_str(), local_ip_str.c_str(), true);

  topic = "devices/";
  topic += Config.hostname;
  topic += "/$ota";
  this->_shared_interface->mqtt->subscribe(topic.c_str(), 1);

  for (int i = 0; i < this->_shared_interface->subscriptions.size(); i++) {
    Subscription subscription = this->_shared_interface->subscriptions[i];

    topic = "devices/";
    topic += Config.hostname;
    topic += "/";
    topic += subscription.node;
    topic += "/";
    topic += subscription.property;
    topic += "/set";
    this->_shared_interface->mqtt->subscribe(topic.c_str(), 1);
  }
}

void BootNormal::_mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = String();
  for (int i = 0; i < length; i++) {
    char input_char = (char)payload[i];
    message += input_char;
  }
  String unified = String(topic);
  unified.remove(0, this->_mqtt_base_topic_length); // Remove /devices/${id}/
  if (unified == "$ota") {
    if (message != this->_shared_interface->version) {
      this->_flagged_for_ota = true;
      Serial.print("Flagged for OTA v.");
      Serial.println(message);
    }
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
  this->_shared_interface->inputHandler(node, property, String(message));
}

void BootNormal::_handleReset() {
  this->_resetDebouncer.update();

  if (this->_resetDebouncer.read() == LOW) {
    Serial.println("Resetting");
    Config.configured = false;
    Config.save();

    this->_shared_interface->resetFunction();

    ESP.restart();
  }
}

void BootNormal::setup() {
  Boot::setup();

  this->_resetDebouncer.attach(PIN_RESET);
  this->_resetDebouncer.interval(5000UL);

  this->_shared_interface->setupFunction();
}

void BootNormal::loop() {
  Boot::loop();

  if (this->_shared_interface->resettable) {
    this->_handleReset();
  }

  this->_shared_interface->readyToOperate = false;

  if (WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - this->_last_wifi_reconnect_attempt > 20000UL || this->_last_wifi_reconnect_attempt == 0) {
      Serial.println("Attempting to connect to WiFi");
      this->_last_wifi_reconnect_attempt = now;
      Blinker.start(LED_WIFI_DELAY);
      this->_wifiConnect();
    }
    return;
  }

  if (!this->_shared_interface->mqtt->connected()) {
    unsigned long now = millis();
    if (now - this->_last_wifi_reconnect_attempt > 5000UL || this->_last_wifi_reconnect_attempt == 0) {
      Serial.println("Attempting to connect to MQTT");
      this->_last_mqtt_reconnect_attempt = now;
      Blinker.start(LED_MQTT_DELAY);
      this->_mqttConnect();
    }
    return;
  } else {
    Blinker.stop();
  }

  this->_shared_interface->readyToOperate = true;
  this->_shared_interface->loopFunction();

  if (this->_flagged_for_ota && this->_shared_interface->resettable) {
    Serial.println("Rebooting in OTA mode");
    Config.boot_mode = BOOT_OTA;
    Config.save();

    ESP.restart();
  }

  this->_shared_interface->mqtt->loop();
}

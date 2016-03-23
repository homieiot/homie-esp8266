#include "MqttClient.hpp"

using namespace HomieInternals;

MqttClient::MqttClient() {
}

MqttClient::~MqttClient() {
  delete[] this->_topicBuffer;
}

void MqttClient::initMqtt(bool secure) {
  if (secure) {
    this->_pubSubClient.setClient(this->_wifiClientSecure);
  } else {
    this->_pubSubClient.setClient(this->_wifiClient);
  }

  this->_pubSubClient.setCallback(std::bind(&MqttClient::_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void MqttClient::initBuffer(unsigned char maxTopicLength) {
  this->_topicBuffer = new char[maxTopicLength];
}

char* MqttClient::getTopicBuffer() {
  return this->_topicBuffer;
}

void MqttClient::setCallback(std::function<void(char* topic, char* message)> callback) {
  _userCallback = callback;
}

void MqttClient::setServer(const char* host, unsigned int port, const char* fingerprint) {
  this->_host = strdup(host);
  this->_fingerprint = strdup(fingerprint);
  this->_pubSubClient.setServer(host, port);
}

bool MqttClient::connect(const char* clientId, const char* willMessage, unsigned char willQos, bool willRetain, bool auth, const char* username, const char* password) {
  bool result;
  if (auth) {
    result = this->_pubSubClient.connect(clientId, username, password, this->_topicBuffer, willQos, willRetain, willMessage);
  } else {
    result = this->_pubSubClient.connect(clientId, this->_topicBuffer, willQos, willRetain, willMessage);
  }

  if (this->_secure && !(strcmp_P(this->_fingerprint, PSTR("")) == 0)) {
    Logger.logln(F("Checking certificate"));
    if(!this->_wifiClientSecure.verify(this->_fingerprint, this->_host)) {
      Logger.logln(F("✖ MQTT SSL certificate mismatch"));
      this->_pubSubClient.disconnect();
      return false;
    }
  }

  return result;
}

void MqttClient::disconnect() {
  this->_pubSubClient.disconnect();
}

bool MqttClient::publish(const char* message, bool retained) {
  return this->_pubSubClient.publish(this->_topicBuffer, message, retained);
}

bool MqttClient::subscribe(unsigned char qos) {
  return this->_pubSubClient.subscribe(this->_topicBuffer, qos);
  this->loop(); // see knolleary/pubsublient#98
}

void MqttClient::loop() {
  this->_pubSubClient.loop();
}

bool MqttClient::connected() {
  return this->_pubSubClient.connected();
}

void MqttClient::_callback(char* topic, byte* payload, unsigned int length) {
  std::unique_ptr<char[]> buf(new char[length + 1]);
  for (int i = 0; i < length; i++) {
    char tempString[2];
    tempString[0] = (char)payload[i];
    tempString[1] = '\0';
    if (i == 0) strcpy(buf.get(), tempString);
    else strcat(buf.get(), tempString);
  }

  this->_userCallback(topic, buf.get());
}

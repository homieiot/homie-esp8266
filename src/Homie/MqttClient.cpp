#include "MqttClient.hpp"

using namespace HomieInternals;

MqttClientClass::MqttClientClass()
: _topicBuffer {'\0'}
, _secure(false)
, _host()
, _port(0)
, _fingerprint()
, _subscribeWithoutLoop(0)
{
}

MqttClientClass::~MqttClientClass() {
}

void MqttClientClass::initMqtt(bool secure) {
  if (secure) {
    this->_pubSubClient.setClient(this->_wifiClientSecure);
  } else {
    this->_pubSubClient.setClient(this->_wifiClient);
  }

  this->_secure = secure;

  this->_pubSubClient.setCallback(std::bind(&MqttClientClass::_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

char* MqttClientClass::getTopicBuffer() {
  return this->_topicBuffer;
}

void MqttClientClass::setCallback(std::function<void(char* topic, char* message)> callback) {
  _userCallback = callback;
}

void MqttClientClass::setServer(const char* host, unsigned int port, const char* fingerprint) {
  this->_host = host;
  this->_port = port;
  this->_fingerprint = fingerprint;
  this->_pubSubClient.setServer(this->_host, port);
}

bool MqttClientClass::connect(const char* clientId, const char* willMessage, unsigned char willQos, bool willRetain, bool auth, const char* username, const char* password) {
  this->_wifiClient.stop(); // Ensure buffers are cleaned, otherwise exception
  this->_wifiClientSecure.stop();

  if (this->_secure && !(strcmp_P(this->_fingerprint, PSTR("")) == 0)) {
    Logger.logln(F("Checking certificate"));
    if (!this->_wifiClientSecure.connect(this->_host, this->_port)) {
      this->_wifiClientSecure.stop();
      return false;
    }

    if (!this->_wifiClientSecure.verify(this->_fingerprint, this->_host)) {
      Logger.logln(F("✖ MQTT SSL certificate mismatch"));
      this->_wifiClientSecure.stop();
      return false;
    }

    this->_wifiClientSecure.stop();
  }

  bool result;
  if (auth) {
    result = this->_pubSubClient.connect(clientId, username, password, this->_topicBuffer, willQos, willRetain, willMessage);
  } else {
    result = this->_pubSubClient.connect(clientId, this->_topicBuffer, willQos, willRetain, willMessage);
  }

  return result;
}

int MqttClientClass::getState() {
  return this->_pubSubClient.state();
}

void MqttClientClass::disconnect() {
  this->_pubSubClient.disconnect();
}

bool MqttClientClass::publish(const char* message, bool retained) {
  return this->_pubSubClient.publish(this->_topicBuffer, message, retained);
}

bool MqttClientClass::subscribe(unsigned char qos) {
  if (this->_subscribeWithoutLoop >= 5) {
    this->loop(); // see knolleary/pubsublient#98
  }

  return this->_pubSubClient.subscribe(this->_topicBuffer, qos);
}

void MqttClientClass::loop() {
  this->_pubSubClient.loop();
  this->_subscribeWithoutLoop = 0;
}

bool MqttClientClass::connected() {
  return this->_pubSubClient.connected();
}

void MqttClientClass::_callback(char* topic, unsigned char* payload, unsigned int length) {
  char buf[128];
  for (unsigned int i = 0; i < length; i++) {
    char tempString[2];
    tempString[0] = (char)payload[i];
    tempString[1] = '\0';
    if (i == 0) strcpy(buf, tempString);
    else strcat(buf, tempString);
  }

  this->_userCallback(topic, buf);
}

MqttClientClass HomieInternals::MqttClient;

#include "MqttClient.hpp"

using namespace HomieInternals;

MqttClient::MqttClient()
: _interface(nullptr)
, _topicBuffer {'\0'}
, _secure(false)
, _host()
, _port(0)
, _fingerprint()
, _subscribeWithoutLoop(0)
{
}

MqttClient::~MqttClient() {
}

void MqttClient::attachInterface(Interface* interface) {
  _interface = interface;
}

void MqttClient::initMqtt(bool secure) {
  if (secure) {
    _pubSubClient.setClient(_wifiClientSecure);
  } else {
    _pubSubClient.setClient(_wifiClient);
  }

  _secure = secure;

  _pubSubClient.setCallback(std::bind(&MqttClient::_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

char* MqttClient::getTopicBuffer() {
  return _topicBuffer;
}

void MqttClient::setCallback(std::function<void(char* topic, char* message)> callback) {
  _userCallback = callback;
}

void MqttClient::setServer(const char* host, uint16_t port, const char* fingerprint) {
  _host = host;
  _port = port;
  _fingerprint = fingerprint;
  _pubSubClient.setServer(_host, port);
}

bool MqttClient::connect(const char* clientId, const char* willMessage, uint8_t willQos, bool willRetain, bool auth, const char* username, const char* password) {
  _wifiClient.stop(); // Ensure buffers are cleaned, otherwise exception
  _wifiClientSecure.stop();

  if (_secure && !(strcmp_P(_fingerprint, PSTR("")) == 0)) {
    _interface->logger->logln(F("Checking certificate"));
    if (!_wifiClientSecure.connect(_host, _port)) {
      _wifiClientSecure.stop();
      return false;
    }

    if (!_wifiClientSecure.verify(_fingerprint, _host)) {
      _interface->logger->logln(F("✖ MQTT SSL certificate mismatch"));
      _wifiClientSecure.stop();
      return false;
    }

    _wifiClientSecure.stop();
  }

  bool result;
  if (auth) {
    result = _pubSubClient.connect(clientId, username, password, _topicBuffer, willQos, willRetain, willMessage);
  } else {
    result = _pubSubClient.connect(clientId, _topicBuffer, willQos, willRetain, willMessage);
  }

  return result;
}

int MqttClient::getState() {
  return _pubSubClient.state();
}

void MqttClient::disconnect() {
  _pubSubClient.disconnect();
}

bool MqttClient::publish(const char* message, bool retained) {
  return _pubSubClient.publish(_topicBuffer, message, retained);
}

bool MqttClient::subscribe(uint8_t qos) {
  if (_subscribeWithoutLoop >= 5) {
    this->loop(); // see knolleary/pubsublient#98
  }

  return _pubSubClient.subscribe(_topicBuffer, qos);
}

void MqttClient::loop() {
  _pubSubClient.loop();
  _subscribeWithoutLoop = 0;
}

bool MqttClient::connected() {
  return _pubSubClient.connected();
}

void MqttClient::_callback(char* topic, uint8_t* payload, uint16_t length) {
  char buf[128];
  for (uint16_t i = 0; i < length; i++) {
    char tempString[2];
    tempString[0] = (char)payload[i];
    tempString[1] = '\0';
    if (i == 0) strcpy(buf, tempString);
    else strcat(buf, tempString);
  }

  _userCallback(topic, buf);
}

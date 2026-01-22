#include "SendingPromise.hpp"
#include "Homie/Limits.hpp"

using namespace HomieInternals;

SendingPromise::SendingPromise()
: _node(nullptr)
, _property(nullptr)
, _qos(0)
, _retained(false)
, _overwriteSetter(false)
, _range { .isRange = false, .index = 0 } {
}

SendingPromise& SendingPromise::setQos(uint8_t qos) {
  _qos = qos;
  return *this;
}

SendingPromise& SendingPromise::setRetained(bool retained) {
  _retained = retained;
  return *this;
}

SendingPromise& SendingPromise::overwriteSetter(bool overwrite) {
  _overwriteSetter = overwrite;
  return *this;
}

SendingPromise& SendingPromise::setRange(const HomieRange& range) {
  _range = range;
  return *this;
}

SendingPromise& SendingPromise::setRange(uint16_t rangeIndex) {
  HomieRange range;
  range.isRange = true;
  range.index = rangeIndex;
  _range = range;
  return *this;
}

uint16_t SendingPromise::send(const String& value) {
  if (!Interface::get().ready) {
    Interface::get().getLogger() << F("✖ setNodeProperty(): impossible now") << endl;
    return 0;
  }

  // Use a local buffer to construct topic - ESP8266/ESP32 is single-threaded
  char topic[MAX_MQTT_TOPIC_LENGTH];
  
  // Build topic: baseTopic + deviceId + "/" + nodeId + ["_" + rangeIndex] + "/" + property [+ "/set"]
  strcpy(topic, Interface::get().getConfig().get().mqtt.baseTopic);
  strcat(topic, Interface::get().getConfig().get().deviceId);
  strcat_P(topic, PSTR("/"));
  strcat(topic, _node->getId());
  
  if (_range.isRange) {
    char rangeStr[6];  // max 65535 = 5 digits + null
    itoa(_range.index, rangeStr, 10);
    strcat_P(topic, PSTR("_"));
    strcat(topic, rangeStr);
    _range.isRange = false;                  //FIXME: This is a workaround. Problem is that Range is loaded from the property into SendingPromise, but the SendingPromise is global. (one SendingPromise for the HomieClass instance
    _range.index = 0;
  }

  strcat_P(topic, PSTR("/"));
  strcat(topic, _property->c_str());

  uint16_t packetId = Interface::get().getMqttClient().publish(topic, _qos, _retained, value.c_str());

  if (_overwriteSetter) {
    strcat_P(topic, PSTR("/set"));
    Interface::get().getMqttClient().publish(topic, 1, true, value.c_str());
  }

  return packetId;
}

SendingPromise& SendingPromise::setNode(const HomieNode& node) {
  _node = &node;
  return *this;
}

SendingPromise& SendingPromise::setProperty(const String& property) {
  _property = &property;
  return *this;
}

const HomieNode* SendingPromise::getNode() const {
  return _node;
}

const String* SendingPromise::getProperty() const {
  return _property;
}

uint8_t SendingPromise::getQos() const {
  return _qos;
}

HomieRange SendingPromise::getRange() const {
  return _range;
}

bool SendingPromise::isRetained() const {
  return _retained;
}

bool SendingPromise::doesOverwriteSetter() const {
  return _overwriteSetter;
}

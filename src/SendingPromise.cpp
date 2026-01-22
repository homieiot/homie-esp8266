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
  int offset = 0;
  
  // Start with baseTopic + deviceId
  offset = snprintf(topic, sizeof(topic), "%s%s/", 
                    Interface::get().getConfig().get().mqtt.baseTopic,
                    Interface::get().getConfig().get().deviceId);
  
  // Add nodeId with optional range
  if (_range.isRange) {
    offset += snprintf(topic + offset, sizeof(topic) - offset, "%s_%u/",
                       _node->getId(), _range.index);
    _range.isRange = false;  //FIXME: This is a workaround. Problem is that Range is loaded from the property into SendingPromise, but the SendingPromise is global. (one SendingPromise for the HomieClass instance
    _range.index = 0;
  } else {
    offset += snprintf(topic + offset, sizeof(topic) - offset, "%s/",
                       _node->getId());
  }
  
  // Add property
  snprintf(topic + offset, sizeof(topic) - offset, "%s", _property->c_str());

  uint16_t packetId = Interface::get().getMqttClient().publish(topic, _qos, _retained, value.c_str());

  if (_overwriteSetter) {
    // Append "/set" for setter overwrite
    size_t topicLen = strlen(topic);
    if (topicLen + 4 < sizeof(topic)) {  // Check space for "/set"
      strcat_P(topic, PSTR("/set"));
      Interface::get().getMqttClient().publish(topic, 1, true, value.c_str());
    }
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

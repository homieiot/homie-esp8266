#include "SendingPromise.hpp"

using namespace HomieInternals;

SendingPromise::SendingPromise()
    : _qos(0), _retained(false), _overwriteSetter(false)
{
}

SendingPromise &SendingPromise::setQos(uint8_t qos)
{
  _qos = qos;
  return *this;
}

SendingPromise &SendingPromise::setRetained(bool retained)
{
  _retained = retained;
  return *this;
}

SendingPromise &SendingPromise::overwriteSetter(bool overwrite)
{
  _overwriteSetter = overwrite;
  return *this;
}

uint16_t SendingPromise::send(const std::string &node_path, const std::string &value)
{
  if (!Interface::get().ready)
  {
    Interface::get().getLogger() << F("✖ setNodeProperty(): impossible now") << endl;
    return 0;
  }
  //Todo changed
  char *topic = new char[strlen(Interface::get().getConfig().get().mqtt.baseTopic) +
                         strlen(Interface::get().getConfig().get().deviceId) + 1 +
                         node_path.size() + 1];
  // char *topic = new char[strlen(Interface::get().getConfig().get().mqtt.baseTopic) +
  // strlen(Interface::get().getConfig().get().deviceId) + 1 +
  // strlen(_node->getName().c_str()) + 1 +
  // strlen(_property->c_str()) + 6 + 5 + 1]; // last + 6 for range _65536, last + 4 for /set
  strcpy(topic, Interface::get().getConfig().get().mqtt.baseTopic);
  strcat(topic, Interface::get().getConfig().get().deviceId);
  strcat_P(topic, PSTR("/"));
  //Todo changed
  strcat(topic, node_path.c_str());
  // strcat(topic, _node->getName().c_str());
  // strcat_P(topic, PSTR("/"));
  // strcat(topic, _property->c_str());

  uint16_t packetId = Interface::get().getMqttClient().publish(topic, _qos, _retained, value.c_str());

  if (_overwriteSetter)
  {
    //directly call handleMsg
    // strcat_P(topic, PSTR("/$set"));
    // Interface::get().getMqttClient().publish(topic, 1, true, value.c_str());
    Binary data(value.length());
    for (int i = 0; i < value.length(); i++)
      data[i] = value[i];
    HomieNode::getMaster()->handleMsg(node_path, data);
  }

  delete[] topic;

  return packetId;
}

uint8_t SendingPromise::getQos() const
{
  return _qos;
}

bool SendingPromise::isRetained() const
{
  return _retained;
}

bool SendingPromise::doesOverwriteSetter() const
{
  return _overwriteSetter;
}

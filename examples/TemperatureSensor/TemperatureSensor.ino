#define DEBUG true

#include <Homie.h>
#include "Nodes/Types/RangeType.hpp"
#include "Nodes/ExecuteNode.hpp"
#include "TemperatureNode.hpp"
const int PIN_RELAY = LED_BUILTIN;

HomieNode HomieNode::MasterHomieNode("master", "master node");

using NodeClass = Node::Node;

Node::TemperatureNode temprature("living_room");

void lightSetup()
{
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
}
bool lightOnHandler(const Node::RangeType<0, 100> &range)
{
  bool on = (range.value < 50);
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  Homie.getLogger() << "Light is " << range.value << endl;
  return true;
}
Node::ExecuteNode<Node::RangeType<0, 100>> light("light", lightOnHandler);
void setup()
{
  Serial.begin(115200);
  Serial << endl
         << endl;
  Homie.setSetupFunction(lightSetup);
  Homie_setFirmware("temprature", "1.0.1");
  HomieNode::getMaster()->addNode(&light);
  HomieNode::getMaster()->addNode(&temprature);
  Homie.setup();
}

void loop()
{
  Homie.loop();
}

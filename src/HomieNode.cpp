#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

std::vector<HomieNode*> HomieNode::nodes;

HomieNode::HomieNode(const char* id, const char* type, NodeInputHandler inputHandler)
: _id(id)
, _type(type)
, _properties()
, _inputHandler(inputHandler) {
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH) {
    Serial.println(F("✖ HomieNode(): either the id or type string is too long"));
    Serial.flush();
    abort();
  }
  Homie._checkBeforeSetup(F("HomieNode::HomieNode"));

  HomieNode::nodes.push_back(this);
}

Property* HomieNode::advertise(const char* property) {
  Property* propertyObject = new Property(property);

  _properties.push_back(propertyObject);

  return propertyObject;
}

bool HomieNode::handleInput(String const &property, String const &value) {
  return _inputHandler(property, value);
}

const std::vector<HomieInternals::Property*>& HomieNode::getProperties() const {
  return _properties;
}

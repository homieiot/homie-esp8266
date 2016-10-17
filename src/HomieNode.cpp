#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

std::vector<HomieNode*> HomieNode::nodes;

PropertyInterface::PropertyInterface()
: _property(nullptr) {
}

void PropertyInterface::settable(PropertyInputHandler inputHandler) {
  _property->settable(inputHandler);
}

PropertyInterface& PropertyInterface::setProperty(Property* property) {
  _property = property;
  return *this;
}

HomieNode::HomieNode(const char* id, const char* type, NodeInputHandler inputHandler)
: _id(id)
, _type(type)
, _properties()
, _inputHandler(inputHandler) {
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH) {
    Serial << F("✖ HomieNode(): either the id or type string is too long") << endl;
    Serial.flush();
    abort();
  }
  Homie._checkBeforeSetup(F("HomieNode::HomieNode"));

  HomieNode::nodes.push_back(this);
}

PropertyInterface& HomieNode::advertise(const char* property) {
  Property* propertyObject = new Property(property);

  _properties.push_back(propertyObject);

  return _propertyInterface.setProperty(propertyObject);
}

PropertyInterface& HomieNode::advertiseRange(const char* property, uint16_t lower, uint16_t upper) {
  Property* propertyObject = new Property(property, true, lower, upper);

  _properties.push_back(propertyObject);

  return _propertyInterface.setProperty(propertyObject);
}

bool HomieNode::handleInput(const String& property, const HomieRange& range, const String& value) {
  return _inputHandler(property, range, value);
}

const std::vector<HomieInternals::Property*>& HomieNode::getProperties() const {
  return _properties;
}

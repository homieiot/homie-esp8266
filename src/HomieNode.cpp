#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

std::vector<HomieNode*> HomieNode::nodes;

PropertyInterface::PropertyInterface()
: _property(nullptr) {
}

PropertyInterface& PropertyInterface::settable(const PropertyInputHandler& inputHandler) {
  _property->settable(inputHandler);
  return *this;
}

PropertyInterface& PropertyInterface::setName(const char* name) {
  _property->setName(name);
  return *this;
}

PropertyInterface& PropertyInterface::setUnit(const char* unit) {
  _property->setUnit(unit);
  return *this;
}

PropertyInterface& PropertyInterface::setDatatype(const char* datatype) {
  _property->setDatatype(datatype);
  return *this;
}

PropertyInterface& PropertyInterface::setFormat(const char* format) {
  _property->setFormat(format);
  return *this;
}

PropertyInterface& PropertyInterface::setRetained(const bool retained) {
  _property->setRetained(retained);
  return *this;
}

PropertyInterface& PropertyInterface::setProperty(Property* property) {
  _property = property;
  return *this;
}

HomieNode::HomieNode(const char* id, const char* name, const char* type, bool range, uint16_t lower, uint16_t upper, const NodeInputHandler& inputHandler)
: _id(id)
, _name(name)
, _type(type)
, _range(range)
, _lower(lower)
, _upper(upper)
, runLoopDisconnected(false)
, _properties()
, _inputHandler(inputHandler) {
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH) {
    Helpers::abort(F("✖ HomieNode(): either the id or type string is too long"));
    return;  // never reached, here for clarity
  }
  Homie._checkBeforeSetup(F("HomieNode::HomieNode"));

  HomieNode::nodes.push_back(this);
}

HomieNode::~HomieNode() {
    Helpers::abort(F("✖✖ ~HomieNode(): Destruction of HomieNode object not possible\n  Hint: Don't create HomieNode objects as a local variable (e.g. in setup())"));
    return;  // never reached, here for clarity
}

PropertyInterface& HomieNode::advertise(const char* id) {
  Property* propertyObject = new Property(id);

  _properties.push_back(propertyObject);

  return _propertyInterface.setProperty(propertyObject);
}

SendingPromise& HomieNode::setProperty(const String& property) const {
  Property* iProperty = this->getProperty(property);
  if (iProperty &&  iProperty->isRetained()) {
      return Interface::get().getSendingPromise().setNode(*this).setProperty(property).setQos(1).setRetained(true);
  } else {
      return Interface::get().getSendingPromise().setNode(*this).setProperty(property).setQos(1);
  }
}

Property* HomieNode::getProperty(const String& property) const {
  for (Property* iProperty : getProperties()) {
    if (strcmp(iProperty->getId(), property.c_str()) == 0)
       return iProperty;
  }
  return NULL;
}

bool HomieNode::handleInput(const HomieRange& range, const String& property, const String& value) {
  return _inputHandler(range, property, value);
}

const std::vector<HomieInternals::Property*>& HomieNode::getProperties() const {
  return _properties;
}

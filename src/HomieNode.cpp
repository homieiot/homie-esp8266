#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

std::vector<HomieNode*> HomieNode::nodes;

PropertyInterface::PropertyInterface()
: _property(nullptr) {
}

void PropertyInterface::settable(const PropertyInputHandler& inputHandler) {
  _property->settable(inputHandler);
}

PropertyInterface& PropertyInterface::setProperty(Property* property) {
  _property = property;
  return *this;
}

HomieNode::HomieNode(const char* id, const char* type, const NodeInputHandler& inputHandler)
: _id(id)
, _type(type)
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

SendingPromise& HomieNode::setProperty(const String& property) const {
  return Interface::get().getSendingPromise().setNode(*this).setProperty(property).setQos(1).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 });
}

bool HomieNode::handleInput(const String& property, const HomieRange& range, const String& value) {
  return _inputHandler(property, range, value);
}

const std::vector<HomieInternals::Property*>& HomieNode::getProperties() const {
  return _properties;
}

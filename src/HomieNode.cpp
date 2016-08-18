#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

std::vector<HomieNode*> HomieNode::nodes;

HomieNode::HomieNode(const char* id, const char* type, NodeInputHandler inputHandler)
: _id(id)
, _type(type)
, _subscribeToAll(false)
, _inputHandler(inputHandler) {
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH) {
    Serial.println(F("✖ HomieNode(): either the id or type string is too long"));
    Serial.flush();
    abort();
  }
  Homie._checkBeforeSetup(F("HomieNode::HomieNode"));

  HomieNode::nodes.push_back(this);
}

void HomieNode::subscribe(const char* property, PropertyInputHandler inputHandler) {
  if (strlen(property) + 1 > MAX_NODE_PROPERTY_LENGTH) {
    Serial.println(F("✖ subscribe(): the property string is too long"));
    Serial.flush();
    abort();
  }

  Subscription subscription;
  strcpy(subscription.property, property);
  subscription.inputHandler = inputHandler;
  _subscriptions.push_back(subscription);
}

void HomieNode::subscribeToAll() {
  _subscribeToAll = true;
}

bool HomieNode::handleInput(String const &property, String const &value) {
  return _inputHandler(property, value);
}

const std::vector<HomieInternals::Subscription>& HomieNode::getSubscriptions() const {
  return _subscriptions;
}

uint8_t HomieNode::getSubscriptionsCount() const {
  return _subscriptions.size();
}

bool HomieNode::isSubscribedToAll() const {
  return _subscribeToAll;
}

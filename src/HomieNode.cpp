#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

HomieNode* HomieNode::_first = nullptr;
HomieNode* HomieNode::_last = nullptr;
uint8_t HomieNode::_nodeCount = 0;

HomieNode::HomieNode(const char* id, const char* type, NodeInputHandler inputHandler)
: _id(id)
, _type(type)
, _subscribeToAll(false)
, _inputHandler(inputHandler)
, _next() {
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH) {
    Serial.println(F("✖ HomieNode(): either the id or type string is too long"));
    Serial.flush();
    abort();
  }
  Homie._checkBeforeSetup(F("HomieNode::HomieNode"));
  if (_last)
    _last->_next = this;
  else
    _first = this;
  _last = this;
  ++_nodeCount;
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

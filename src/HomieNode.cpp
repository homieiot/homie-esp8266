#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

HomieNode* HomieNode::_first = 0;
HomieNode* HomieNode::_last = 0;
unsigned HomieNode::_nodeCount = 0;

HomieNode::HomieNode(const char* id, const char* type, NodeInputHandler inputHandler, bool subscribeToAll)
: _id(id)
, _type(type)
, _subscriptionsCount(0)
, _subscribeToAll(subscribeToAll)
, _inputHandler(inputHandler)
, _next() {
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH) {
    Serial.println(F("✖ HomieNode(): either the id or type string is too long"));
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
    abort();
  }

  if (this->_subscriptionsCount > MAX_SUBSCRIPTIONS_COUNT_PER_NODE) {
    Serial.println(F("✖ subscribe(): the max subscription count has been reached"));
    abort();
  }

  Subscription subscription;
  strcpy(subscription.property, property);
  subscription.inputHandler = inputHandler;
  this->_subscriptions[this->_subscriptionsCount++] = subscription;
}

bool HomieNode::handleInput(String const &property, String const &value) {
  return this->_inputHandler(property, value);
}

const Subscription* HomieNode::getSubscriptions() const {
  return this->_subscriptions;
}

unsigned char HomieNode::getSubscriptionsCount() const {
  return this->_subscriptionsCount;
}

bool HomieNode::getSubscribeToAll() const {
  return this->_subscribeToAll;
}

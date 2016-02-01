#include "HomieNode.h"

using namespace HomieInternals;

HomieNode::HomieNode(const char* id, const char* type, bool (*callback)(String property, String message)) {
  this->id = strdup(id);
  this->type = strdup(type);
  this->inputHandler = callback;
}

void HomieNode::subscribe(const char* property, bool (*callback)(String message)) {
  Subscription subscription;
  subscription.property = strdup(property);
  subscription.inputHandler = callback;
  this->subscriptions.push_back(subscription);
}

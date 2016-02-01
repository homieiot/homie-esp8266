#include "HomieNode.h"

using namespace HomieInternals;

HomieNode::HomieNode(const char* id, const char* type) {
  this->id = strdup(id);
  this->type = strdup(type);
}

void HomieNode::subscribe(const char* property) {
  Subscription subscription;
  subscription.property = strdup(property);
  this->subscriptions.push_back(subscription);
}

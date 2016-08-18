#pragma once

#include <functional>
#include <vector>
#include "Arduino.h"
#include "Homie/Datatypes/Subscription.hpp"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/Limits.hpp"

namespace HomieInternals {
class HomieClass;
class BootNormal;
class BootConfig;
}

class HomieNode {
  friend HomieInternals::HomieClass;
  friend HomieInternals::BootNormal;
  friend HomieInternals::BootConfig;

 public:
  HomieNode(const char* id, const char* type, HomieInternals::NodeInputHandler nodeInputHandler = [](String property, String value) { return false; });

  const char* getId() const { return _id; }
  const char* getType() const { return _type; }

  void subscribe(const char* property, HomieInternals::PropertyInputHandler inputHandler = [](String value) { return false; });
  void subscribeToAll();

 protected:
  virtual void setup() {}
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual bool handleInput(String const &property, String const &value);

 private:
  const std::vector<HomieInternals::Subscription>& getSubscriptions() const;
  uint8_t getSubscriptionsCount() const;
  bool isSubscribedToAll() const;

  static HomieNode* find(const char* id) {
    for (HomieNode* iNode : HomieNode::nodes) {
      if (strcmp(id, iNode->getId()) == 0) return iNode;
    }

    return 0;
  }

  const char* _id;
  const char* _type;
  std::vector<HomieInternals::Subscription> _subscriptions;
  bool _subscribeToAll;
  HomieInternals::NodeInputHandler _inputHandler;

  static std::vector<HomieNode*> nodes;
};

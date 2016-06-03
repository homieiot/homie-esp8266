#pragma once

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
    HomieNode(const char* id, const char* type, HomieInternals::NodeInputHandler nodeInputHandler = [](String property, String value) { return false; }, bool subscribeToAll = false);

    const char* getId() const { return this->_id; }
    const char* getType() const { return this->_type; }

    void subscribe(const char* property, HomieInternals::PropertyInputHandler inputHandler = [](String value) { return false; });

    static void for_each(std::function<void(HomieNode *)> f) {
      for (HomieNode *n = _first; n; n = n->_next)
        f(n);
    }

    static HomieNode *find(String const &id) {
      for (HomieNode *n = _first; n; n = n->_next)
        if (id == n->getId())
          return n;
      return 0;
    }

    static unsigned getNodeCount() {
      return _nodeCount;
    }

  protected:
    virtual void setup() {}

    virtual void loop() {}

    virtual void onReadyToOperate() {}

    virtual bool handleInput(String const &property, String const &value);

  private:
    const HomieInternals::Subscription* getSubscriptions() const;
    unsigned char getSubscriptionsCount() const;
    bool getSubscribeToAll() const;

    const char* _id;
    const char* _type;
    HomieInternals::Subscription _subscriptions[HomieInternals::MAX_SUBSCRIPTIONS_COUNT_PER_NODE];
    unsigned char _subscriptionsCount;
    bool _subscribeToAll;
    HomieInternals::NodeInputHandler _inputHandler;
    HomieNode* _next;
    static HomieNode *_first, *_last;
    static unsigned _nodeCount;
};

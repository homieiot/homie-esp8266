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

    void subscribe(const char* property, HomieInternals::PropertyInputHandler inputHandler = [](String value) { return false; });

  protected:
    virtual void setup() {}

    virtual void loop() {}

    virtual void onReadyToOperate() {}

    virtual bool handleInput(String const &property, String const &value);

  private:
    const char* getType() const;
    const HomieInternals::Subscription* getSubscriptions() const;
    unsigned char getSubscriptionsCount() const;
    bool getSubscribeToAll() const;

    const char* _id;
    const char* _type;
    HomieInternals::Subscription _subscriptions[HomieInternals::MAX_SUBSCRIPTIONS_COUNT_PER_NODE];
    unsigned char _subscriptionsCount;
    bool _subscribeToAll;
    HomieInternals::NodeInputHandler _inputHandler;
};

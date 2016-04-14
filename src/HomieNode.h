#ifndef HomieNode_h
#define HomieNode_h

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

    void subscribe(const char* property, HomieInternals::PropertyInputHandler inputHandler = [](String value) { return false; });

  private:
    const char* getId();
    const char* getType();
    HomieInternals::Subscription* getSubscriptions();
    unsigned char getSubscriptionsCount();
    HomieInternals::NodeInputHandler getInputHandler();

    const char* _id;
    const char* _type;
    HomieInternals::Subscription _subscriptions[HomieInternals::MAX_SUBSCRIPTIONS_COUNT_PER_NODE];
    unsigned char _subscriptionsCount;
    HomieInternals::NodeInputHandler _inputHandler;
};

#endif

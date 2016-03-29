#ifndef HomieNode_h
#define HomieNode_h

#include "Arduino.h"
#include "Homie/Datatypes/Subscription.hpp"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/Limits.hpp"

class HomieNode {
  public:
    HomieNode(const char* id, const char* type, HomieInternals::NodeInputHandler nodeInputHandler = [](String property, String value) { return false; });

    void subscribe(const char* property, HomieInternals::PropertyInputHandler inputHandler = [](String value) { return false; });

    const char* getId();
    const char* getType();
    HomieInternals::Subscription* getSubscriptions();
    unsigned char getSubscriptionsCount();
    HomieInternals::NodeInputHandler getInputHandler();

  private:
    const char* _id;
    const char* _type;
    HomieInternals::Subscription _subscriptions[HomieInternals::MAX_SUBSCRIPTIONS_COUNT_PER_NODE];
    unsigned char _subscriptionsCount;
    HomieInternals::NodeInputHandler _inputHandler;
};

#endif

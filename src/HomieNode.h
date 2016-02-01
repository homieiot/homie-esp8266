#ifndef HomieNode_h
#define HomieNode_h

#include <Arduino.h>
#include <vector>
#include "Homie/Datatypes/Subscription.hpp"

class HomieNode {
  public:
    HomieNode(const char* id, const char* type);

    void subscribe(const char* property);

    char* id;
    char* type;
    std::vector<HomieInternals::Subscription> subscriptions;
};

#endif

#ifndef HomieNode_h
#define HomieNode_h

#include <Arduino.h>
#include <vector>
#include "Homie/Datatypes/Subscription.hpp"

class HomieNode {
  public:
    HomieNode(const char* id, const char* type, bool (*callback)(String property, String message) = [](String property, String message) { return false; });

    void subscribe(const char* property, bool (*callback)(String message) = [](String message) { return false; });

    char* id;
    char* type;
    bool (*inputHandler)(String property, String message);
    std::vector<HomieInternals::Subscription> subscriptions;
};

#endif

#pragma once

#include <vector>
#include <PubSubClient.h>
#include "../../HomieNode.h"
#include "../../HomieEvent.h"

namespace HomieInternals {
  struct SharedInterface {
    /***** User configurable data *****/
    char* brand;
    struct {
      char* name;
      char* version;
    } firmware;
    std::vector<HomieNode> registeredNodes;
    bool useBuiltInLed;
    // Reset
    bool resettable;
    bool resetTriggerEnabled;
    uint8_t resetTriggerPin;
    byte resetTriggerState;
    uint16_t resetTriggerTime;
    bool (*resetFunction)(void);
    // Callbacks
    bool (*inputHandler)(String node, String property, String message);
    void (*setupFunction)(void);
    void (*loopFunction)(void);
    void (*eventHandler)(HomieEvent event);

    /***** Runtime data *****/
    bool readyToOperate;
    PubSubClient* mqtt;
  };
}

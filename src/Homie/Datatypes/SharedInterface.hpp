#pragma once

#include <vector>
#include <PubSubClient.h>
#include "../../HomieNode.h"

namespace HomieInternals {
  struct SharedInterface {
    char* fwname;
    char* fwversion;
    bool resettable;
    bool readyToOperate;
    std::vector<HomieNode> nodes;
    bool (*inputHandler)(String node, String property, String message);
    void (*setupFunction)(void);
    void (*loopFunction)(void);
    void (*resetHook)(void);

    bool resetTriggerEnabled;
    uint8_t resetTriggerPin;
    byte resetTriggerState;
    uint16_t resetTriggerTime;

    bool (*resetFunction)(void);

    PubSubClient* mqtt;
  };
}

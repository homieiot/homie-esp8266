#pragma once

#include <vector>
#include "../../3rd/PubSubClient/src/PubSubClient.h"
#include "../../HomieNode.h"

namespace HomieInternals {
  struct SharedInterface {
    int eepromCount;
    char* fwname;
    char* fwversion;
    bool resettable;
    bool readyToOperate;
    std::vector<HomieNode> nodes;
    bool (*inputHandler)(String node, String property, String message);
    void (*setupFunction)(void);
    void (*loopFunction)(void);
    void (*resetFunction)(void);
    PubSubClient* mqtt;
  };
}

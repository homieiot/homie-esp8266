#pragma once

#include <vector>
#include <PubSubClient.h>
#include "../../HomieNode.h"
#include "../../HomieEvent.h"

namespace HomieInternals {
  struct Interface {
    /***** User configurable data *****/
    char* brand;

    struct Firmware {
      char* name;
      char* version;
    } firmware;

    struct LED {
      bool enable;
      uint8_t pin;
      byte on;
    } led;

    struct Reset {
      bool enable;
      bool able;
      uint8_t triggerPin;
      byte triggerState;
      uint16_t triggerTime;
      bool (*userFunction)(void);
    } reset;

    std::vector<HomieNode> registeredNodes;

    bool (*inputHandler)(String node, String property, String message);
    void (*setupFunction)(void);
    void (*loopFunction)(void);
    void (*eventHandler)(HomieEvent event);

    /***** Runtime data *****/
    bool readyToOperate;
    PubSubClient* mqtt;
  };
}

#pragma once

#include "../Limits.hpp"
#include "./Callbacks.hpp"
#include "../../HomieNode.hpp"
#include "../../HomieEvent.hpp"

namespace HomieInternals {
  class Logger;
  class Blinker;
  class Config;
  class MqttClient;
  struct Interface {
    /***** User configurable data *****/
    char brand[MAX_BRAND_LENGTH];

    struct Firmware {
      char name[MAX_FIRMWARE_NAME_LENGTH];
      char version[MAX_FIRMWARE_VERSION_LENGTH];
    } firmware;

    struct LED {
      bool enabled;
      unsigned char pin;
      unsigned char on;
    } led;

    struct Reset {
      bool enabled;
      bool able;
      unsigned char triggerPin;
      unsigned char triggerState;
      unsigned int triggerTime;
      ResetFunction userFunction;
    } reset;

    const HomieNode* registeredNodes[MAX_REGISTERED_NODES_COUNT];
    unsigned char registeredNodesCount;

    GlobalInputHandler globalInputHandler;
    OperationFunction setupFunction;
    OperationFunction loopFunction;
    EventHandler eventHandler;

    /***** Runtime data *****/
    bool readyToOperate;
    Logger* logger;
    Blinker* blinker;
    Config* config;
    MqttClient* mqttClient;
  };
}

#pragma once

#include <AsyncMqttClient.h>
#include "../Limits.hpp"
#include "./Callbacks.hpp"
#include "../../HomieNode.hpp"
#include "../../HomieEvent.hpp"

namespace HomieInternals {
class Logger;
class Blinker;
class Config;
struct Interface {
  /***** User configurable data *****/
  char brand[MAX_BRAND_LENGTH];

  bool standalone;

  struct Firmware {
    char name[MAX_FIRMWARE_NAME_LENGTH];
    char version[MAX_FIRMWARE_VERSION_LENGTH];
  } firmware;

  struct LED {
    bool enabled;
    uint8_t pin;
    uint8_t on;
  } led;

  struct Reset {
    bool enabled;
    bool able;
    uint8_t triggerPin;
    uint8_t triggerState;
    uint16_t triggerTime;
    ResetFunction userFunction;
  } reset;

  GlobalInputHandler globalInputHandler;
  BroadcastHandler broadcastHandler;
  OperationFunction setupFunction;
  OperationFunction loopFunction;
  EventHandler eventHandler;

  /***** Runtime data *****/
  bool connected;
  Logger* logger;
  Blinker* blinker;
  Config* config;
  AsyncMqttClient* mqttClient;
};
}  // namespace HomieInternals

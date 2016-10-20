#pragma once

#include <AsyncMqttClient.h>
#include "../Logger.hpp"
#include "../Blinker.hpp"
#include "../Config.hpp"
#include "../Limits.hpp"
#include "./Callbacks.hpp"
#include "../../HomieNode.hpp"
#include "../../HomieEvent.hpp"

namespace HomieInternals {
class Logger;
class Blinker;
class Config;
class HomieClass;
class InterfaceData {
 friend HomieClass;
 public:
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
    bool idle;
    uint8_t triggerPin;
    uint8_t triggerState;
    uint16_t triggerTime;
    bool flaggedBySketch;
  } reset;

  GlobalInputHandler globalInputHandler;
  BroadcastHandler broadcastHandler;
  OperationFunction setupFunction;
  OperationFunction loopFunction;
  EventHandler eventHandler;

  /***** Runtime data *****/
  HomieEvent event;
  bool connected;
  Logger& getLogger() { return *_logger; }
  Blinker& getBlinker() { return *_blinker; }
  Config& getConfig() { return *_config; }
  AsyncMqttClient& getMqttClient() { return *_mqttClient; }
 private:
   Logger* _logger;
   Blinker* _blinker;
   Config* _config;
   AsyncMqttClient* _mqttClient;
};

class Interface {
 public:
  static InterfaceData& get();

 private:
  static InterfaceData _interface;
};
}  // namespace HomieInternals

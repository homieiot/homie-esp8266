#pragma once

#include "Arduino.h"

#include <AsyncMqttClient.h>
#include "Homie/Datatypes/Interface.hpp"
#include "Homie/Constants.hpp"
#include "Homie/Limits.hpp"
#include "Homie/Utils/DeviceId.hpp"
#include "Homie/Boot/Boot.hpp"
#include "Homie/Boot/BootStandalone.hpp"
#include "Homie/Boot/BootNormal.hpp"
#include "Homie/Boot/BootConfig.hpp"
#include "Homie/Logger.hpp"
#include "Homie/Config.hpp"
#include "Homie/Blinker.hpp"

#include "SendingPromise.hpp"
#include "HomieNode.hpp"
#include "HomieSetting.hpp"
#include "StreamingOperator.hpp"

#define Homie_setFirmware(name, version) const char* __FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" name "\x93\x44\x6b\xa7\x75"; const char* __FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" version "\xb0\x30\x48\xd4\x1a"; Homie.__setFirmware(__FLAGGED_FW_NAME, __FLAGGED_FW_VERSION);
#define Homie_setBrand(brand) const char* __FLAGGED_BRAND = "\xfb\x2a\xf5\x68\xc0" brand "\x6e\x2f\x0f\xeb\x2d"; Homie.__setBrand(__FLAGGED_BRAND);

namespace HomieInternals {
class HomieClass {
  friend class ::HomieNode;
  friend SendingPromise;

 public:
  HomieClass();
  ~HomieClass();
  void setup();
  void loop();

  void __setFirmware(const char* name, const char* version);
  void __setBrand(const char* brand) const;

  HomieClass& disableLogging();
  HomieClass& setLoggingPrinter(Print* printer);
  HomieClass& disableLedFeedback();
  HomieClass& setLedPin(uint8_t pin, uint8_t on);
  HomieClass& setGlobalInputHandler(GlobalInputHandler globalInputHandler);
  HomieClass& setBroadcastHandler(BroadcastHandler broadcastHandler);
  HomieClass& onEvent(EventHandler handler);
  HomieClass& setResetTrigger(uint8_t pin, uint8_t state, uint16_t time);
  HomieClass& disableResetTrigger();
  HomieClass& setSetupFunction(OperationFunction function);
  HomieClass& setLoopFunction(OperationFunction function);
  HomieClass& setStandalone();

  void reset();
  void setIdle(bool idle);
  bool isConfigured() const;
  bool isConnected() const;
  const ConfigStruct& getConfiguration() const;
  AsyncMqttClient& getMqttClient();
  Logger& getLogger();
  void prepareToSleep();

 private:
  bool _setupCalled;
  bool _firmwareSet;
  Boot* _boot;
  BootStandalone _bootStandalone;
  BootNormal _bootNormal;
  BootConfig _bootConfig;
  SendingPromise _sendingPromise;
  Logger _logger;
  Blinker _blinker;
  Config _config;
  AsyncMqttClient _mqttClient;

  void _checkBeforeSetup(const __FlashStringHelper* functionName) const;

  const char* __HOMIE_SIGNATURE;
};
}  // namespace HomieInternals

extern HomieInternals::HomieClass Homie;

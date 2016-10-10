#pragma once

#include <AsyncMqttClient.h>
#include "Homie/Blinker.hpp"
#include "Homie/Logger.hpp"
#include "Homie/Config.hpp"
#include "Homie/Constants.hpp"
#include "Homie/Limits.hpp"
#include "Homie/Helpers.hpp"
#include "Homie/Boot/Boot.hpp"
#include "Homie/Boot/BootStandalone.hpp"
#include "Homie/Boot/BootNormal.hpp"
#include "Homie/Boot/BootConfig.hpp"

#include "HomieNode.hpp"
#include "HomieSetting.hpp"
#include "StreamingOperator.hpp"

#define Homie_setFirmware(name, version) const char* __FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" name "\x93\x44\x6b\xa7\x75"; const char* __FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" version "\xb0\x30\x48\xd4\x1a"; Homie.__setFirmware(__FLAGGED_FW_NAME, __FLAGGED_FW_VERSION);
#define Homie_setBrand(brand) const char* __FLAGGED_BRAND = "\xfb\x2a\xf5\x68\xc0" brand "\x6e\x2f\x0f\xeb\x2d"; Homie.__setBrand(__FLAGGED_BRAND);

namespace HomieInternals {
class HomieClass;

class SendingPromise {
  friend HomieClass;

 public:
  explicit SendingPromise(HomieClass* homie);
  SendingPromise& setQos(uint8_t qos);
  SendingPromise& setRetained(bool retained);
  SendingPromise& setRange(HomieRange range);
  SendingPromise& setRange(uint16_t rangeIndex);
  uint16_t send(const String& value);

 private:
  SendingPromise& setNode(const HomieNode& node);
  SendingPromise& setProperty(const String& property);
  const HomieNode* getNode() const;
  const String* getProperty() const;
  uint8_t getQos() const;
  HomieRange getRange() const;
  bool isRetained() const;

  HomieClass* _homie;
  const HomieNode* _node;
  const String* _property;
  uint8_t _qos;
  bool _retained;
  HomieRange _range;
};

class HomieClass {
  friend class ::HomieNode;
  friend SendingPromise;

 public:
  HomieClass();
  ~HomieClass();
  void setup();
  void loop();

  void __setFirmware(const char* name, const char* version);
  void __setBrand(const char* brand);

  HomieClass& disableLogging();
  HomieClass& setLoggingPrinter(Print* printer);
  HomieClass& disableLedFeedback();
  HomieClass& setLedPin(uint8_t pin, uint8_t on);
  HomieClass& setGlobalInputHandler(GlobalInputHandler globalInputHandler);
  HomieClass& onEvent(EventHandler handler);
  HomieClass& setResetTrigger(uint8_t pin, uint8_t state, uint16_t time);
  HomieClass& disableResetTrigger();
  HomieClass& setResetFunction(ResetFunction function);
  HomieClass& setSetupFunction(OperationFunction function);
  HomieClass& setLoopFunction(OperationFunction function);
  HomieClass& setStandalone();

  SendingPromise& setNodeProperty(const HomieNode& node, const String& property) {
    return _sendingPromise.setNode(node).setProperty(property).setQos(1).setRetained(true).setRange({ .isRange = false, .index = 0 });
  }

  void setIdle(bool idle);
  void eraseConfiguration();
  bool isConfigured() const;
  bool isConnected() const;
  const ConfigStruct& getConfiguration() const;
  AsyncMqttClient& getMqttClient();
  void prepareToSleep();

 private:
  bool _setupCalled;
  Boot* _boot;
  BootStandalone _bootStandalone;
  BootNormal _bootNormal;
  BootConfig _bootConfig;
  SendingPromise _sendingPromise;
  Interface _interface;
  Logger _logger;
  Blinker _blinker;
  Config _config;
  AsyncMqttClient _mqttClient;

  void _checkBeforeSetup(const __FlashStringHelper* functionName) const;

  const char* __HOMIE_SIGNATURE;
};
}  // namespace HomieInternals

extern HomieInternals::HomieClass Homie;

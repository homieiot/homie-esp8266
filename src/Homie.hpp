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

#define Homie_setFirmware(name, version) const char* __FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" name "\x93\x44\x6b\xa7\x75"; const char* __FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" version "\xb0\x30\x48\xd4\x1a"; Homie.__setFirmware(__FLAGGED_FW_NAME, __FLAGGED_FW_VERSION);
#define Homie_setBrand(brand) const char* __FLAGGED_BRAND = "\xfb\x2a\xf5\x68\xc0" brand "\x6e\x2f\x0f\xeb\x2d"; Homie.__setBrand(__FLAGGED_BRAND);

namespace HomieInternals {
class HomieClass {
  friend class ::HomieNode;

 public:
  HomieClass();
  ~HomieClass();
  void setup();
  void loop();

  void __setFirmware(const char* name, const char* version);
  void __setBrand(const char* brand);

  void enableLogging(bool enable);
  void setLoggingPrinter(Print* printer);
  void enableBuiltInLedIndicator(bool enable);
  void setLedPin(uint8_t pin, uint8_t on);
  void setGlobalInputHandler(GlobalInputHandler globalInputHandler);
  void setResettable(bool resettable);
  void onEvent(EventHandler handler);
  void setResetTrigger(uint8_t pin, uint8_t state, uint16_t time);
  void disableResetTrigger();
  void setResetFunction(ResetFunction function);
  void eraseConfig();
  void setSetupFunction(OperationFunction function);
  void setLoopFunction(OperationFunction function);
  void setStandalone();
  bool isConfigured() const;
  bool isConnected() const;
  void setNodeProperty(const HomieNode& node, const String& property, const String& value, uint8_t qos = 1, bool retained = true) {
    setNodeProperty(node, property.c_str(), value.c_str(), qos, retained);
  }
  void setNodeProperty(const HomieNode& node, const char* property, const char* value, uint8_t qos = 1, bool retained = true);
  const ConfigStruct& getConfiguration() const;
  AsyncMqttClient& getMqttClient();

 private:
  bool _setupCalled;
  Boot* _boot;
  BootStandalone _bootStandalone;
  BootNormal _bootNormal;
  BootConfig _bootConfig;
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

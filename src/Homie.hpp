#pragma once

#include "Homie/MqttClient.hpp"
#include "Homie/Blinker.hpp"
#include "Homie/Logger.hpp"
#include "Homie/Config.hpp"
#include "Homie/Constants.hpp"
#include "Homie/Limits.hpp"
#include "Homie/Helpers.hpp"
#include "Homie/Boot/Boot.hpp"
#include "Homie/Boot/BootNormal.hpp"
#include "Homie/Boot/BootConfig.hpp"
#include "Homie/Boot/BootOta.hpp"

#include "HomieNode.hpp"

namespace HomieInternals {
  class HomieClass {
    public:
      HomieClass();
      ~HomieClass();
      void setup();
      void loop();

      void enableLogging(bool enable);
      void enableBuiltInLedIndicator(bool enable);
      void setLedPin(unsigned char pin, unsigned char on);
      void setBrand(const char* name);
      void setFirmware(const char* name, const char* version);
      void registerNode(const HomieNode& node);
      void setGlobalInputHandler(GlobalInputHandler globalInputHandler);
      void setResettable(bool resettable);
      void onEvent(EventHandler handler);
      void setResetTrigger(unsigned char pin, unsigned char state, unsigned int time);
      void disableResetTrigger();
      void setResetFunction(ResetFunction function);
      void setSetupFunction(OperationFunction function);
      void setLoopFunction(OperationFunction function);
      bool isReadyToOperate();
      bool setNodeProperty(const HomieNode& node, const String& property, const String& value, bool retained = true) {
        return this->setNodeProperty(node, property.c_str(), value.c_str(), retained);
      }
      bool setNodeProperty(const HomieNode& node, const char* property, const char* value, bool retained = true);
    private:
      bool _setupCalled;
      Boot* _boot;
      BootNormal _bootNormal;
      BootConfig _bootConfig;
      BootOta _bootOta;
      Interface _interface;
      Logger _logger;
      Blinker _blinker;
      Config _config;
      MqttClient _mqttClient;

      void _checkBeforeSetup(const __FlashStringHelper* functionName);
  };
}

extern HomieInternals::HomieClass Homie;

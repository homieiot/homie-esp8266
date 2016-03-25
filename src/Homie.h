#ifndef Homie_h
#define Homie_h

#include <Arduino.h>
#include "Homie/Logger.hpp"
#include "Homie/Config.hpp"
#include "Homie/Datatypes/ConfigStruct.hpp"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/MqttClient.hpp"
#include "Homie/Constants.hpp"
#include "Homie/Limits.hpp"
#include "Homie/Helpers.hpp"
#include "Homie/Boot/Boot.hpp"
#include "Homie/Boot/BootNormal.hpp"
#include "Homie/Boot/BootConfig.hpp"
#include "Homie/Boot/BootOta.hpp"

#include "HomieNode.h"

namespace HomieInternals {
  class HomieClass {
    public:
      HomieClass();
      ~HomieClass();
      void setup();
      void loop();

      void enableLogging(bool logging);
      void enableBuiltInLedIndicator(bool enable);
      void setLedPin(uint8_t pin, byte on);
      void setBrand(const char* name);
      void setFirmware(const char* name, const char* version);
      void registerNode(HomieNode& node);
      void setGlobalInputHandler(GlobalInputHandler globalInputHandler);
      void setResettable(bool resettable);
      void onEvent(EventHandler handler);
      void setResetTrigger(uint8_t pin, byte state, uint16_t time);
      void disableResetTrigger();
      void setResetFunction(ResetFunction function);
      void setSetupFunction(OperationFunction function);
      void setLoopFunction(OperationFunction function);
      bool isReadyToOperate();
      bool setNodeProperty(HomieNode& node, const String& property, const String& value, bool retained = true) {
        return this->setNodeProperty(node, property.c_str(), value.c_str(), retained);
      }
      bool setNodeProperty(HomieNode& node, const char* property, const char* value, bool retained = true);
    private:
      Boot* _boot;
      BootNormal _bootNormal;
      BootConfig _bootConfig;
      BootOta _bootOta;
      Interface _interface;
  };
}

extern HomieInternals::HomieClass Homie;

#endif

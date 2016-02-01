#ifndef Homie_h
#define Homie_h

#include <Arduino.h>
#include "Homie/Logger.hpp"
#include "Homie/Config.hpp"
#include "Homie/Datatypes/ConfigStruct.hpp"
#include "Homie/Constants.hpp"
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

      void setLogging(bool logging);
      void setFirmware(const char* name, const char* version);
      void registerNode(HomieNode& node);
      void setInputHandler(void (*callback)(String node, String property, String message));
      void setResettable(bool resettable);
      void setResetFunction(void (*callback)(void));
      void setSetupFunction(void (*callback)(void));
      void setLoopFunction(void (*callback)(void));
      bool readyToOperate();
      bool sentNodeProperty(HomieNode& node, const char* property, const char* value, bool retained = true) {
        this->setNodeProperty(node, String(property), String(value), retained);
      }
      bool setNodeProperty(HomieNode& node, String property, String value, bool retained = true);
      void reserveEeprom(int bytes);
      int getEepromOffset();
    private:
      Boot* _boot;
      SharedInterface _shared_interface;
  };
}

extern HomieInternals::HomieClass Homie;

#endif

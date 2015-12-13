#ifndef Homie_h
#define Homie_h

#include <Arduino.h>
#include "Homie/Config.hpp"
#include "Homie/Constants.hpp"
#include "Homie/Boot.hpp"
#include "Homie/BootNormal.hpp"
#include "Homie/BootConfig.hpp"
#include "Homie/BootOta.hpp"

class HomieClass {
  public:
    HomieClass();
    ~HomieClass();
    void setup();
    void loop();

    void setVersion(const char* version);
    void addNode(const char* name, const char* type);
    void addSubscription(const char* node, const char* property);
    void setInputHandler(void (*callback)(String node, String property, String message));
    void setResettable(bool resettable);
    void setResetFunction(void (*callback)(void));
    void setSetupFunction(void (*callback)(void));
    void setLoopFunction(void (*callback)(void));
    bool readyToOperate();
    bool sendProperty(const char* node, const char* property, const char* value, bool retained = true) {
      this->sendProperty(String(node), String(property), String(value), retained);
    }
    bool sendProperty(String node, String property, String value, bool retained = true);
    void reserveEeprom(int bytes);
    int getEepromOffset();
  private:
    Boot* _boot;
    SharedInterface _shared_interface;
};

extern HomieClass Homie;

#endif

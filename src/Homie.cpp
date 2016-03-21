#include "Homie.h"

using namespace HomieInternals;

HomieClass::HomieClass() {
  this->_interface.brand = strdup(DEFAULT_BRAND);
  this->_interface.firmware.name = strdup(DEFAULT_FW_NAME);
  this->_interface.firmware.version = strdup(DEFAULT_FW_VERSION);
  this->_interface.led.enable = true;
  this->_interface.led.pin = BUILTIN_LED;
  this->_interface.led.on = LOW;
  this->_interface.reset.able = true;
  this->_interface.reset.enable = true;
  this->_interface.reset.triggerPin = DEFAULT_RESET_PIN;
  this->_interface.reset.triggerState = DEFAULT_RESET_STATE;
  this->_interface.reset.triggerTime = DEFAULT_RESET_TIME;
  this->_interface.reset.userFunction = [](void) { return false; };
  this->_interface.inputHandler = [](String node, String property, String message) { return false; };
  this->_interface.setupFunction = [](void) {};
  this->_interface.loopFunction = [](void) {};
  this->_interface.eventHandler = [](HomieEvent event) {};
  this->_interface.readyToOperate = false;

  Blinker.attachInterface(&this->_interface);
}

HomieClass::~HomieClass() {
  delete this->_boot;
}

void HomieClass::setup(void) {
  if (Logger.isEnabled()) {
    Serial.begin(BAUD_RATE);
  }

  if (!Config.load()) {
    this->_boot = new BootConfig(&this->_interface);
    this->_interface.eventHandler(HOMIE_CONFIGURATION_MODE);
  } else {
    switch (Config.getBootMode()) {
      case BOOT_NORMAL:
        this->_boot = new BootNormal(&this->_interface);
        this->_interface.eventHandler(HOMIE_NORMAL_MODE);
        break;
      case BOOT_OTA:
        this->_boot = new BootOta(&this->_interface);
        this->_interface.eventHandler(HOMIE_OTA_MODE);
        break;
    }
  }

  this->_boot->setup();
}

void HomieClass::loop(void) {
  this->_boot->loop();
}

void HomieClass::enableLogging(bool logging) {
  Logger.setLogging(logging);
}

void HomieClass::enableBuiltInLedIndicator(bool enable) {
  this->_interface.led.enable = enable;
}

void HomieClass::setLedPin(uint8_t pin, byte on) {
  this->_interface.led.pin = pin;
  this->_interface.led.on = on;
}

void HomieClass::setFirmware(const char* name, const char* version) {
  this->_interface.firmware.name = strdup(name);
  this->_interface.firmware.version = strdup(version);
}

void HomieClass::setBrand(const char* name) {
  this->_interface.brand = strdup(name);
}

void HomieClass::registerNode(const HomieNode& node) {
  this->_interface.registeredNodes.push_back(node);
}

bool HomieClass::isReadyToOperate() {
  return this->_interface.readyToOperate;
}

void HomieClass::setResettable(bool resettable) {
  this->_interface.reset.able = resettable;
}

void HomieClass::setGlobalInputHandler(bool (*callback)(String node, String property, String message)) {
  this->_interface.inputHandler = callback;
}

void HomieClass::setResetFunction(bool (*callback)()) {
  this->_interface.reset.userFunction = callback;
}

void HomieClass::setSetupFunction(void (*callback)()) {
  this->_interface.setupFunction = callback;
}

void HomieClass::setLoopFunction(void (*callback)()) {
  this->_interface.loopFunction = callback;
}

void HomieClass::onEvent(void (*callback)(HomieEvent event)) {
  this->_interface.eventHandler = callback;
}

void HomieClass::setResetTrigger(uint8_t pin, byte state, uint16_t time) {
  this->_interface.reset.enable = true;
  this->_interface.reset.triggerPin = pin;
  this->_interface.reset.triggerState = state;
  this->_interface.reset.triggerTime = time;
}

void HomieClass::disableResetTrigger() {
  this->_interface.reset.enable = false;
}

bool HomieClass::setNodeProperty(const HomieNode& node, const char* property, const char* value, bool retained) {
  if (!this->isReadyToOperate()) {
    Logger.logln("setNodeProperty() impossible now");
    return false;
  }

  String topic;
  topic = Config.get().mqtt.baseTopic;
  topic += Helpers.getDeviceId();
  topic += "/";
  topic += node.id;
  topic += "/";
  topic += property;
  return this->_interface.mqtt->publish(topic.c_str(), value, retained);
}

HomieClass Homie;

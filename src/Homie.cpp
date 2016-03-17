#include "Homie.h"

using namespace HomieInternals;

HomieClass::HomieClass() {
  this->_sharedInterface.brand = strdup(DEFAULT_BRAND);
  this->_sharedInterface.firmware.name = strdup(DEFAULT_FW_NAME);
  this->_sharedInterface.firmware.version = strdup(DEFAULT_FW_VERSION);
  this->_sharedInterface.resettable = true;
  this->_sharedInterface.readyToOperate = false;
  this->_sharedInterface.useBuiltInLed = true;
  this->_sharedInterface.inputHandler = [](String node, String property, String message) { return false; };
  this->_sharedInterface.setupFunction = [](void) {};
  this->_sharedInterface.loopFunction = [](void) {};
  this->_sharedInterface.eventHandler = [](HomieEvent event) {};
  this->_sharedInterface.resetTriggerEnabled = true;
  this->_sharedInterface.resetTriggerPin = DEFAULT_RESET_PIN;
  this->_sharedInterface.resetTriggerState = DEFAULT_RESET_STATE;
  this->_sharedInterface.resetTriggerTime = DEFAULT_RESET_TIME;
  this->_sharedInterface.resetFunction = [](void) { return false; };
}

HomieClass::~HomieClass() {
  delete this->_boot;
}

void HomieClass::setup(void) {
  if (Logger.isEnabled()) {
    Serial.begin(BAUD_RATE);
  }

  if (!Config.load()) {
    this->_boot = new BootConfig(&this->_sharedInterface);
    this->_sharedInterface.eventHandler(HOMIE_CONFIGURATION_MODE);
  } else {
    switch (Config.getBootMode()) {
      case BOOT_NORMAL:
        this->_boot = new BootNormal(&this->_sharedInterface);
        this->_sharedInterface.eventHandler(HOMIE_NORMAL_MODE);
        break;
      case BOOT_OTA:
        this->_boot = new BootOta(&this->_sharedInterface);
        this->_sharedInterface.eventHandler(HOMIE_OTA_MODE);
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
  this->_sharedInterface.useBuiltInLed = enable;
}

void HomieClass::setFirmware(const char* name, const char* version) {
  this->_sharedInterface.firmware.name = strdup(name);
  this->_sharedInterface.firmware.version = strdup(version);
}

void HomieClass::setBrand(const char* name) {
  this->_sharedInterface.brand = strdup(name);
}

void HomieClass::registerNode(const HomieNode& node) {
  this->_sharedInterface.registeredNodes.push_back(node);
}

bool HomieClass::isReadyToOperate() {
  return this->_sharedInterface.readyToOperate;
}

void HomieClass::setResettable(bool resettable) {
  this->_sharedInterface.resettable = resettable;
}

void HomieClass::setGlobalInputHandler(bool (*callback)(String node, String property, String message)) {
  this->_sharedInterface.inputHandler = callback;
}

void HomieClass::setResetFunction(bool (*callback)()) {
  this->_sharedInterface.resetFunction = callback;
}

void HomieClass::setSetupFunction(void (*callback)()) {
  this->_sharedInterface.setupFunction = callback;
}

void HomieClass::setLoopFunction(void (*callback)()) {
  this->_sharedInterface.loopFunction = callback;
}

void HomieClass::onEvent(void (*callback)(HomieEvent event)) {
  this->_sharedInterface.eventHandler = callback;
}

void HomieClass::setResetTrigger(uint8_t pin, byte state, uint16_t time) {
  this->_sharedInterface.resetTriggerEnabled = true;
  this->_sharedInterface.resetTriggerPin = pin;
  this->_sharedInterface.resetTriggerState = state;
  this->_sharedInterface.resetTriggerTime = time;
}

void HomieClass::disableResetTrigger() {
  this->_sharedInterface.resetTriggerEnabled = false;
}

bool HomieClass::setNodeProperty(const HomieNode& node, const char* property, const char* value, bool retained) {
  if (!this->isReadyToOperate()) {
    return false;
  }

  String topic;
  topic.reserve(7 + 1 + 8 + 1 + strlen(node.id) + 1 + strlen(property) + 1);
  topic = "devices/";
  topic += Helpers.getDeviceId();
  topic += "/";
  topic += node.id;
  topic += "/";
  topic += property;
  return this->_sharedInterface.mqtt->publish(topic.c_str(), value, retained);
}

HomieClass Homie;

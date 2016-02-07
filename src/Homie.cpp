#include "Homie.h"

using namespace HomieInternals;

HomieClass::HomieClass() {
  this->_shared_interface.fwname = strdup(DEFAULT_FW_NAME);
  this->_shared_interface.fwversion = strdup(DEFAULT_FW_VERSION);
  this->_shared_interface.resettable = true;
  this->_shared_interface.readyToOperate = false;
  this->_shared_interface.inputHandler = [](String node, String property, String message) { return false; };
  this->_shared_interface.setupFunction = [](void) {};
  this->_shared_interface.loopFunction = [](void) {};
  this->_shared_interface.resetHook = [](void) {};
  this->_shared_interface.resetTriggerEnabled = true;
  this->_shared_interface.resetTriggerPin = DEFAULT_RESET_PIN;
  this->_shared_interface.resetTriggerState = DEFAULT_RESET_STATE;
  this->_shared_interface.resetTriggerTime = DEFAULT_RESET_TIME;
  this->_shared_interface.resetFunction = [](void) { return false; };
}

HomieClass::~HomieClass() {
  delete this->_boot;
}

void HomieClass::setup(void) {
  Serial.begin(BAUD_RATE);

  if (!Config.load()) {
    this->_boot = new BootConfig(&this->_shared_interface);
  } else {
    switch (Config.getBootMode()) {
      case BOOT_NORMAL:
        this->_boot = new BootNormal(&this->_shared_interface);
        break;
      case BOOT_OTA:
        this->_boot = new BootOta(&this->_shared_interface);
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

void HomieClass::setFirmware(const char* name, const char* version) {
  this->_shared_interface.fwname = strdup(name);
  this->_shared_interface.fwversion = strdup(version);
}

void HomieClass::registerNode(HomieNode& node) {
  this->_shared_interface.nodes.push_back(node);
}

bool HomieClass::isReadyToOperate() {
  return this->_shared_interface.readyToOperate;
}

void HomieClass::setResettable(bool resettable) {
  this->_shared_interface.resettable = resettable;
}

void HomieClass::setGlobalInputHandler(bool (*callback)(String node, String property, String message)) {
  this->_shared_interface.inputHandler = callback;
}

void HomieClass::setResetFunction(bool (*callback)()) {
  this->_shared_interface.resetFunction = callback;
}

void HomieClass::setSetupFunction(void (*callback)()) {
  this->_shared_interface.setupFunction = callback;
}

void HomieClass::setLoopFunction(void (*callback)()) {
  this->_shared_interface.loopFunction = callback;
}

void HomieClass::setResetHook(void (*callback)(void)) {
  this->_shared_interface.resetHook = callback;
}

void HomieClass::setResetTrigger(uint8_t pin, byte state, uint16_t time) {
  this->_shared_interface.resetTriggerEnabled = true;
  this->_shared_interface.resetTriggerPin = pin;
  this->_shared_interface.resetTriggerState = state;
  this->_shared_interface.resetTriggerTime = time;
}

void HomieClass::disableResetTrigger() {
  this->_shared_interface.resetTriggerEnabled = false;
}

bool HomieClass::setNodeProperty(HomieNode& node, const char* property, const char* value, bool retained) {
  if (!this->isReadyToOperate()) {
    return false;
  }

  String topic = "devices/";
  topic += Helpers.getDeviceId();
  topic += "/";
  topic += node.id;
  topic += "/";
  topic += property;
  return this->_shared_interface.mqtt->publish(topic.c_str(), value, retained);
}

HomieClass Homie;

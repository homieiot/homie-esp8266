#include "Homie.h"

using namespace HomieInternals;

HomieClass::HomieClass() {
  this->_shared_interface.fwname = strdup("undefined");
  this->_shared_interface.fwversion = strdup("undefined");
  this->_shared_interface.resettable = true;
  this->_shared_interface.readyToOperate = false;
  this->_shared_interface.inputHandler = [](String node, String property, String message) { return false; };
  this->_shared_interface.setupFunction = [](void) {};
  this->_shared_interface.loopFunction = [](void) {};
  this->_shared_interface.resetHook = [](void) {};
}

HomieClass::~HomieClass() {
  delete this->_boot;
}

void HomieClass::setup(void) {
  Serial.begin(BAUD_RATE);

  if (!Config.load()) {
    this->_boot = new BootConfig();
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

void HomieClass::setSetupFunction(void (*callback)()) {
  this->_shared_interface.setupFunction = callback;
}

void HomieClass::setLoopFunction(void (*callback)()) {
  this->_shared_interface.loopFunction = callback;
}

void HomieClass::setResetHook(void (*callback)(void)) {
  this->_shared_interface.resetHook = callback;
}

bool HomieClass::setNodeProperty(HomieNode& node, String property, String value, bool retained) {
  if (!this->isReadyToOperate()) {
    return false;
  }

  String topic = "devices/";
  topic += Helpers.getDeviceId();
  topic += "/";
  topic += node.id;
  topic += "/";
  topic += property;
  return this->_shared_interface.mqtt->publish(topic.c_str(), value.c_str(), retained);
}

HomieClass Homie;

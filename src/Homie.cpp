#include "Homie.h"

using namespace HomieInternals;

HomieClass::HomieClass() {
  this->_shared_interface.eepromCount = 0;
  this->_shared_interface.fwname = strdup("undefined");
  this->_shared_interface.fwversion = strdup("undefined");
  this->_shared_interface.resettable = true;
  this->_shared_interface.readyToOperate = false;
  this->_shared_interface.inputHandler = [](String node, String property, String message) {};
  this->_shared_interface.setupFunction = [](void) {};
  this->_shared_interface.loopFunction = [](void) {};
  this->_shared_interface.resetFunction = [](void) {};
}

HomieClass::~HomieClass() {
  delete this->_boot;
}

void HomieClass::setup(void) {
  Config.setCustomEepromSize(this->_shared_interface.eepromCount);

  if (!Config.load()) {
    this->_boot = new BootConfig();
  } else {
    switch (Config.get().boot_mode) {
      case BOOT_NORMAL:
        this->_boot = new BootNormal(&this->_shared_interface);
        break;
      case BOOT_CONFIG:
        this->_boot = new BootConfig();
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

void HomieClass::setLogging(bool logging) {
  Logger.setLogging(logging);
}

void HomieClass::setFirmware(const char* name, const char* version) {
  this->_shared_interface.fwname = strdup(name);
  this->_shared_interface.fwversion = strdup(version);
}

void HomieClass::addNode(const char* name, const char* type) {
  Node node;
  node.name = strdup(name);
  node.type = strdup(type);
  this->_shared_interface.nodes.push_back(node);
}

void HomieClass::addSubscription(const char* node, const char* property) {
  Subscription subscription;
  subscription.node = strdup(node);
  subscription.property = strdup(property);
  this->_shared_interface.subscriptions.push_back(subscription);
}

bool HomieClass::readyToOperate() {
  return this->_shared_interface.readyToOperate;
}

void HomieClass::setResettable(bool resettable) {
  this->_shared_interface.resettable = resettable;
}

void HomieClass::setInputHandler(void (*callback)(String node, String property, String message)) {
  this->_shared_interface.inputHandler = callback;
}

void HomieClass::setSetupFunction(void (*callback)()) {
  this->_shared_interface.setupFunction = callback;
}

void HomieClass::setLoopFunction(void (*callback)()) {
  this->_shared_interface.loopFunction = callback;
}

void HomieClass::setResetFunction(void (*callback)(void)) {
  this->_shared_interface.resetFunction = callback;
}

bool HomieClass::sendProperty(String node, String property, String value, bool retained) {
  if (!this->readyToOperate()) {
    return false;
  }

  String topic = "devices/";
  topic += Helpers::getDeviceId();
  topic += "/";
  topic += node;
  topic += "/";
  topic += property;
  return this->_shared_interface.mqtt->publish(topic.c_str(), value.c_str(), retained);
}


void HomieClass::reserveEeprom(int bytes) {
  this->_shared_interface.eepromCount = bytes;
}

int HomieClass::getEepromOffset() {
  return EEPROM_CONFIG_SIZE + 1;
}

HomieClass Homie;

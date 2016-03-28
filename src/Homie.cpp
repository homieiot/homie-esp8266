#include "Homie.h"

using namespace HomieInternals;

HomieClass::HomieClass() {
  strcpy(this->_interface.brand, DEFAULT_BRAND);
  strcpy(this->_interface.firmware.name, DEFAULT_FW_NAME);
  strcpy(this->_interface.firmware.version, DEFAULT_FW_VERSION);
  this->_interface.led.enable = true;
  this->_interface.led.pin = BUILTIN_LED;
  this->_interface.led.on = LOW;
  this->_interface.reset.able = true;
  this->_interface.reset.enable = true;
  this->_interface.reset.triggerPin = DEFAULT_RESET_PIN;
  this->_interface.reset.triggerState = DEFAULT_RESET_STATE;
  this->_interface.reset.triggerTime = DEFAULT_RESET_TIME;
  this->_interface.reset.userFunction = []() { return false; };
  this->_interface.globalInputHandler = [](String node, String property, String message) { return false; };
  this->_interface.setupFunction = []() {};
  this->_interface.loopFunction = []() {};
  this->_interface.eventHandler = [](HomieEvent event) {};
  this->_interface.readyToOperate = false;

  Helpers::generateDeviceId();

  Blinker.attachInterface(&this->_interface);

  this->_bootNormal.attachInterface(&this->_interface);
  this->_bootOta.attachInterface(&this->_interface);
  this->_bootConfig.attachInterface(&this->_interface);
}

HomieClass::~HomieClass() {
}

void HomieClass::setup() {
  if (Logger.isEnabled()) {
    Serial.begin(BAUD_RATE);
  }

  if (!Config.load()) {
    this->_boot = &this->_bootConfig;
    this->_interface.eventHandler(HOMIE_CONFIGURATION_MODE);
  } else {
    switch (Config.getBootMode()) {
      case BOOT_NORMAL:
        this->_boot = &this->_bootNormal;
        this->_interface.eventHandler(HOMIE_NORMAL_MODE);
        break;
      case BOOT_OTA:
        this->_boot = &this->_bootOta;
        this->_interface.eventHandler(HOMIE_OTA_MODE);
        break;
    }
  }

  this->_boot->setup();
}

void HomieClass::loop() {
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
  if (strlen(name) + 1 > MAX_FIRMWARE_NAME_LENGTH || strlen(version) + 1 > MAX_FIRMWARE_VERSION_LENGTH) {
    Logger.logln(F("setFirmware(): either the name or version string is too long"));
    abort();
  }

  strcpy(this->_interface.firmware.name, name);
  strcpy(this->_interface.firmware.version, version);
}

void HomieClass::setBrand(const char* name) {
  if (strlen(name) + 1 > MAX_BRAND_LENGTH) {
    Logger.logln(F("setBrand(): the brand string is too long"));
    abort();
  }

  strcpy(this->_interface.brand, name);
}

void HomieClass::registerNode(HomieNode& node) {
  if (this->_interface.registeredNodesCount > MAX_REGISTERED_NODES_COUNT) {
    Serial.println(F("register(): the max registered nodes count has been reached"));
    abort();
  }

  this->_interface.registeredNodes[this->_interface.registeredNodesCount++] = &node;
}

bool HomieClass::isReadyToOperate() {
  return this->_interface.readyToOperate;
}

void HomieClass::setResettable(bool resettable) {
  this->_interface.reset.able = resettable;
}

void HomieClass::setGlobalInputHandler(GlobalInputHandler inputHandler) {
  this->_interface.globalInputHandler = inputHandler;
}

void HomieClass::setResetFunction(ResetFunction function) {
  this->_interface.reset.userFunction = function;
}

void HomieClass::setSetupFunction(OperationFunction function) {
  this->_interface.setupFunction = function;
}

void HomieClass::setLoopFunction(OperationFunction function) {
  this->_interface.loopFunction = function;
}

void HomieClass::onEvent(EventHandler handler) {
  this->_interface.eventHandler = handler;
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

bool HomieClass::setNodeProperty(HomieNode& node, const char* property, const char* value, bool retained) {
  if (!this->isReadyToOperate()) {
    Logger.logln(F("setNodeProperty() impossible now"));
    return false;
  }

  strcpy(MqttClient.getTopicBuffer(), Config.get().mqtt.baseTopic);
  strcat(MqttClient.getTopicBuffer(), Config.get().deviceId);
  strcat_P(MqttClient.getTopicBuffer(), PSTR("/"));
  strcat(MqttClient.getTopicBuffer(), node.getId());
  strcat_P(MqttClient.getTopicBuffer(), PSTR("/"));
  strcat(MqttClient.getTopicBuffer(), property);

  return MqttClient.publish(value, retained);
}

HomieClass Homie;

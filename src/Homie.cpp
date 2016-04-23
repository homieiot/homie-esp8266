#include "Homie.hpp"

using namespace HomieInternals;

HomieClass::HomieClass() : _setupCalled(false) {
  strcpy(this->_interface.brand, DEFAULT_BRAND);
  strcpy(this->_interface.firmware.name, DEFAULT_FW_NAME);
  strcpy(this->_interface.firmware.version, DEFAULT_FW_VERSION);
  this->_interface.led.enabled = true;
  this->_interface.led.pin = BUILTIN_LED;
  this->_interface.led.on = LOW;
  this->_interface.reset.able = true;
  this->_interface.reset.enabled = true;
  this->_interface.reset.triggerPin = DEFAULT_RESET_PIN;
  this->_interface.reset.triggerState = DEFAULT_RESET_STATE;
  this->_interface.reset.triggerTime = DEFAULT_RESET_TIME;
  this->_interface.reset.userFunction = []() { return false; };
  this->_interface.globalInputHandler = [](String node, String property, String value) { return false; };
  this->_interface.setupFunction = []() {};
  this->_interface.loopFunction = []() {};
  this->_interface.eventHandler = [](HomieEvent event) {};
  this->_interface.readyToOperate = false;
  this->_interface.logger = &this->_logger;
  this->_interface.blinker = &this->_blinker;
  this->_interface.config = &this->_config;
  this->_interface.mqttClient = &this->_mqttClient;

  Helpers::generateDeviceId();

  this->_config.attachInterface(&this->_interface);
  this->_blinker.attachInterface(&this->_interface);
  this->_mqttClient.attachInterface(&this->_interface);

  this->_bootNormal.attachInterface(&this->_interface);
  this->_bootOta.attachInterface(&this->_interface);
  this->_bootConfig.attachInterface(&this->_interface);
}

HomieClass::~HomieClass() {
}

void HomieClass::_checkBeforeSetup(const __FlashStringHelper* functionName) {
  if (_setupCalled) {
    this->_logger.log(F("✖ "));
    this->_logger.log(functionName);
    this->_logger.logln(F("(): has to be called before setup()"));
    abort();
  }
}

void HomieClass::setup() {
  _setupCalled = true;

  if (this->_logger.isEnabled()) {
    Serial.begin(BAUD_RATE);
    this->_logger.logln();
    this->_logger.logln();
  }

  if (!this->_config.load()) {
    this->_boot = &this->_bootConfig;
    this->_logger.logln(F("Triggering HOMIE_CONFIGURATION_MODE event..."));
    this->_interface.eventHandler(HOMIE_CONFIGURATION_MODE);
  } else {
    switch (this->_config.getBootMode()) {
      case BOOT_NORMAL:
        this->_boot = &this->_bootNormal;
        this->_logger.logln(F("Triggering HOMIE_NORMAL_MODE event..."));
        this->_interface.eventHandler(HOMIE_NORMAL_MODE);
        break;
      case BOOT_OTA:
        this->_boot = &this->_bootOta;
        this->_logger.logln(F("Triggering HOMIE_OTA_MODE event..."));
        this->_interface.eventHandler(HOMIE_OTA_MODE);
        break;
      default:
        this->_logger.logln(F("✖ The boot mode is invalid"));
        abort();
        break;
    }
  }

  this->_boot->setup();
}

void HomieClass::loop() {
  this->_boot->loop();
}

void HomieClass::enableLogging(bool enable) {
  this->_checkBeforeSetup(F("enableLogging"));

  this->_logger.setLogging(enable);
}

void HomieClass::enableBuiltInLedIndicator(bool enable) {
  this->_checkBeforeSetup(F("enableBuiltInLedIndicator"));

  this->_interface.led.enabled = enable;
}

void HomieClass::setLedPin(unsigned char pin, unsigned char on) {
  this->_checkBeforeSetup(F("setLedPin"));

  this->_interface.led.pin = pin;
  this->_interface.led.on = on;
}

void HomieClass::setFirmware(const char* name, const char* version) {
  this->_checkBeforeSetup(F("setFirmware"));
  if (strlen(name) + 1 > MAX_FIRMWARE_NAME_LENGTH || strlen(version) + 1 > MAX_FIRMWARE_VERSION_LENGTH) {
    this->_logger.logln(F("✖ setFirmware(): either the name or version string is too long"));
    abort();
  }

  strcpy(this->_interface.firmware.name, name);
  strcpy(this->_interface.firmware.version, version);
}

void HomieClass::setBrand(const char* name) {
  this->_checkBeforeSetup(F("setBrand"));
  if (strlen(name) + 1 > MAX_BRAND_LENGTH) {
    this->_logger.logln(F("✖ setBrand(): the brand string is too long"));
    abort();
  }

  strcpy(this->_interface.brand, name);
}

void HomieClass::registerNode(const HomieNode& node) {
  this->_checkBeforeSetup(F("registerNode"));
  if (this->_interface.registeredNodesCount > MAX_REGISTERED_NODES_COUNT) {
    Serial.println(F("✖ register(): the max registered nodes count has been reached"));
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
  this->_checkBeforeSetup(F("setGlobalInputHandler"));

  this->_interface.globalInputHandler = inputHandler;
}

void HomieClass::setResetFunction(ResetFunction function) {
  this->_checkBeforeSetup(F("setResetFunction"));

  this->_interface.reset.userFunction = function;
}

void HomieClass::setSetupFunction(OperationFunction function) {
  this->_checkBeforeSetup(F("setSetupFunction"));

  this->_interface.setupFunction = function;
}

void HomieClass::setLoopFunction(OperationFunction function) {
  this->_checkBeforeSetup(F("setLoopFunction"));

  this->_interface.loopFunction = function;
}

void HomieClass::onEvent(EventHandler handler) {
  this->_checkBeforeSetup(F("onEvent"));

  this->_interface.eventHandler = handler;
}

void HomieClass::setResetTrigger(unsigned char pin, unsigned char state, unsigned int time) {
  this->_checkBeforeSetup(F("setResetTrigger"));

  this->_interface.reset.enabled = true;
  this->_interface.reset.triggerPin = pin;
  this->_interface.reset.triggerState = state;
  this->_interface.reset.triggerTime = time;
}

void HomieClass::disableResetTrigger() {
  this->_checkBeforeSetup(F("disableResetTrigger"));

  this->_interface.reset.enabled = false;
}

bool HomieClass::setNodeProperty(const HomieNode& node, const char* property, const char* value, bool retained) {
  if (!this->isReadyToOperate()) {
    this->_logger.logln(F("✖ setNodeProperty(): impossible now"));
    return false;
  }

  strcpy(this->_mqttClient.getTopicBuffer(), this->_config.get().mqtt.baseTopic);
  strcat(this->_mqttClient.getTopicBuffer(), this->_config.get().deviceId);
  strcat_P(this->_mqttClient.getTopicBuffer(), PSTR("/"));
  strcat(this->_mqttClient.getTopicBuffer(), node.getId());
  strcat_P(this->_mqttClient.getTopicBuffer(), PSTR("/"));
  strcat(this->_mqttClient.getTopicBuffer(), property);

  if (5 + 2 + strlen(this->_mqttClient.getTopicBuffer()) + strlen(value) + 1 > MQTT_MAX_PACKET_SIZE) {
    this->_logger.logln(F("✖ setNodeProperty(): content to send is too long"));
    return false;
  }

  return this->_mqttClient.publish(value, retained);
}

HomieClass Homie;

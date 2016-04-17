#include "Homie.hpp"

using namespace HomieInternals;

HomieClass::HomieClass() : _setup(false) {
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

  Helpers::generateDeviceId();

  this->_bootNormal.attachInterface(&this->_interface);
  this->_bootOta.attachInterface(&this->_interface);
  this->_bootConfig.attachInterface(&this->_interface);
}

HomieClass::~HomieClass() {
}

void HomieClass::_checkBeforeSetup(const __FlashStringHelper* functionName) {
  if (_setup) {
    Logger.log(F("✖ "));
    Logger.log(functionName);
    Logger.logln(F("(): has to be called before setup()"));
    abort();
  }
}

void HomieClass::setup() {
  Blinker.attachInterface(&this->_interface); // here otherwise in constructor this crashes because Blinker might not be constructed

  if (Logger.isEnabled()) {
    Serial.begin(BAUD_RATE);
    Logger.logln();
    Logger.logln();
  }

  _setup = true;

  if (!Config.load()) {
    this->_boot = &this->_bootConfig;
    Logger.logln(F("Triggering HOMIE_CONFIGURATION_MODE event..."));
    this->_interface.eventHandler(HOMIE_CONFIGURATION_MODE);
  } else {
    switch (Config.getBootMode()) {
      case BOOT_NORMAL:
        this->_boot = &this->_bootNormal;
        Logger.logln(F("Triggering HOMIE_NORMAL_MODE event..."));
        this->_interface.eventHandler(HOMIE_NORMAL_MODE);
        break;
      case BOOT_OTA:
        this->_boot = &this->_bootOta;
        Logger.logln(F("Triggering HOMIE_OTA_MODE event..."));
        this->_interface.eventHandler(HOMIE_OTA_MODE);
        break;
      default:
        Logger.logln(F("✖ The boot mode is invalid"));
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

  Logger.setLogging(enable);
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
    Logger.logln(F("✖ setFirmware(): either the name or version string is too long"));
    abort();
  }

  strcpy(this->_interface.firmware.name, name);
  strcpy(this->_interface.firmware.version, version);
}

void HomieClass::setBrand(const char* name) {
  this->_checkBeforeSetup(F("setBrand"));
  if (strlen(name) + 1 > MAX_BRAND_LENGTH) {
    Logger.logln(F("✖ setBrand(): the brand string is too long"));
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
    Logger.logln(F("✖ setNodeProperty(): impossible now"));
    return false;
  }

  strcpy(MqttClient.getTopicBuffer(), Config.get().mqtt.baseTopic);
  strcat(MqttClient.getTopicBuffer(), Config.get().deviceId);
  strcat_P(MqttClient.getTopicBuffer(), PSTR("/"));
  strcat(MqttClient.getTopicBuffer(), node.getId());
  strcat_P(MqttClient.getTopicBuffer(), PSTR("/"));
  strcat(MqttClient.getTopicBuffer(), property);

  if (5 + 2 + strlen(MqttClient.getTopicBuffer()) + strlen(value) + 1 > MQTT_MAX_PACKET_SIZE) {
    Logger.logln(F("✖ setNodeProperty(): content to send is too long"));
    return false;
  }

  return MqttClient.publish(value, retained);
}

HomieClass Homie;

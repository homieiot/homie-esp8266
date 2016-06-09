#include "Homie.hpp"

using namespace HomieInternals;

HomieClass::HomieClass() : _setupCalled(false) {
  strcpy(_interface.brand, DEFAULT_BRAND);
  strcpy(_interface.firmware.name, DEFAULT_FW_NAME);
  strcpy(_interface.firmware.version, DEFAULT_FW_VERSION);
  _interface.led.enabled = true;
  _interface.led.pin = BUILTIN_LED;
  _interface.led.on = LOW;
  _interface.reset.able = true;
  _interface.reset.enabled = true;
  _interface.reset.triggerPin = DEFAULT_RESET_PIN;
  _interface.reset.triggerState = DEFAULT_RESET_STATE;
  _interface.reset.triggerTime = DEFAULT_RESET_TIME;
  _interface.reset.userFunction = []() { return false; };
  _interface.globalInputHandler = [](String node, String property, String value) { return false; };
  _interface.setupFunction = []() {};
  _interface.loopFunction = []() {};
  _interface.eventHandler = [](HomieEvent event) {};
  _interface.readyToOperate = false;
  _interface.logger = &_logger;
  _interface.blinker = &_blinker;
  _interface.config = &_config;
  _interface.mqttClient = &_mqttClient;

  Helpers::generateDeviceId();

  _config.attachInterface(&_interface);
  _blinker.attachInterface(&_interface);
  _mqttClient.attachInterface(&_interface);

  _bootNormal.attachInterface(&_interface);
  _bootOta.attachInterface(&_interface);
  _bootConfig.attachInterface(&_interface);
}

HomieClass::~HomieClass() {
}

void HomieClass::_checkBeforeSetup(const __FlashStringHelper* functionName) const {
  if (_setupCalled) {
    _logger.log(F("✖ "));
    _logger.log(functionName);
    _logger.logln(F("(): has to be called before setup()"));
    abort();
  }
}

void HomieClass::setup() {
  _setupCalled = true;

  if (!_config.load()) {
    _boot = &_bootConfig;
    _logger.logln(F("Triggering HOMIE_CONFIGURATION_MODE event..."));
    _interface.eventHandler(HOMIE_CONFIGURATION_MODE);
  } else {
    switch (_config.getBootMode()) {
      case BOOT_NORMAL:
        _boot = &_bootNormal;
        _logger.logln(F("Triggering HOMIE_NORMAL_MODE event..."));
        _interface.eventHandler(HOMIE_NORMAL_MODE);
        break;
      case BOOT_OTA:
        _boot = &_bootOta;
        _logger.logln(F("Triggering HOMIE_OTA_MODE event..."));
        _interface.eventHandler(HOMIE_OTA_MODE);
        break;
      default:
        _logger.logln(F("✖ The boot mode is invalid"));
        abort();
        break;
    }
  }

  _boot->setup();
}

void HomieClass::loop() {
  _boot->loop();
}

void HomieClass::enableLogging(bool enable) {
  _checkBeforeSetup(F("enableLogging"));

  _logger.setLogging(enable);
}

void HomieClass::setLoggingPrinter(Print* printer) {
  _checkBeforeSetup(F("setLoggingPrinter"));

  _logger.setPrinter(printer);
}

void HomieClass::enableBuiltInLedIndicator(bool enable) {
  _checkBeforeSetup(F("enableBuiltInLedIndicator"));

  _interface.led.enabled = enable;
}

void HomieClass::setLedPin(uint8_t pin, uint8_t on) {
  _checkBeforeSetup(F("setLedPin"));

  _interface.led.pin = pin;
  _interface.led.on = on;
}

void HomieClass::__setFirmware(const char* name, const char* version) {
  _checkBeforeSetup(F("setFirmware"));
  if (strlen(name) + 1 - 10 > MAX_FIRMWARE_NAME_LENGTH || strlen(version) + 1 - 10 > MAX_FIRMWARE_VERSION_LENGTH) {
    _logger.logln(F("✖ setFirmware(): either the name or version string is too long"));
    abort();
  }

  strncpy(_interface.firmware.name, name + 5, strlen(name) - 10);
  _interface.firmware.name[strlen(name) - 10] = '\0';
  strncpy(_interface.firmware.version, version + 5, strlen(version) - 10);
  _interface.firmware.version[strlen(version) - 10] = '\0';
}

void HomieClass::__setBrand(const char* brand) {
  _checkBeforeSetup(F("setBrand"));
  if (strlen(brand) + 1 - 10 > MAX_BRAND_LENGTH) {
    _logger.logln(F("✖ setBrand(): the brand string is too long"));
    abort();
  }

  strncpy(_interface.brand, brand + 5, strlen(brand) - 10);
  _interface.brand[strlen(brand) - 10] = '\0';
}

bool HomieClass::isReadyToOperate() const {
  return _interface.readyToOperate;
}

void HomieClass::setResettable(bool resettable) {
  _interface.reset.able = resettable;
}

void HomieClass::setGlobalInputHandler(GlobalInputHandler inputHandler) {
  _checkBeforeSetup(F("setGlobalInputHandler"));

  _interface.globalInputHandler = inputHandler;
}

void HomieClass::setResetFunction(ResetFunction function) {
  _checkBeforeSetup(F("setResetFunction"));

  _interface.reset.userFunction = function;
}

void HomieClass::setSetupFunction(OperationFunction function) {
  _checkBeforeSetup(F("setSetupFunction"));

  _interface.setupFunction = function;
}

void HomieClass::setLoopFunction(OperationFunction function) {
  _checkBeforeSetup(F("setLoopFunction"));

  _interface.loopFunction = function;
}

void HomieClass::onEvent(EventHandler handler) {
  _checkBeforeSetup(F("onEvent"));

  _interface.eventHandler = handler;
}

void HomieClass::setResetTrigger(uint8_t pin, uint8_t state, uint16_t time) {
  _checkBeforeSetup(F("setResetTrigger"));

  _interface.reset.enabled = true;
  _interface.reset.triggerPin = pin;
  _interface.reset.triggerState = state;
  _interface.reset.triggerTime = time;
}

void HomieClass::disableResetTrigger() {
  _checkBeforeSetup(F("disableResetTrigger"));

  _interface.reset.enabled = false;
}

void HomieClass::eraseConfig() {
  this->_config.erase();
}

bool HomieClass::setNodeProperty(const HomieNode& node, const char* property, const char* value, bool retained) {
  if (!this->isReadyToOperate()) {
    _logger.logln(F("✖ setNodeProperty(): impossible now"));
    return false;
  }

  strcpy(_mqttClient.getTopicBuffer(), _config.get().mqtt.baseTopic);
  strcat(_mqttClient.getTopicBuffer(), _config.get().deviceId);
  strcat_P(_mqttClient.getTopicBuffer(), PSTR("/"));
  strcat(_mqttClient.getTopicBuffer(), node.getId());
  strcat_P(_mqttClient.getTopicBuffer(), PSTR("/"));
  strcat(_mqttClient.getTopicBuffer(), property);

  if (5 + 2 + strlen(_mqttClient.getTopicBuffer()) + strlen(value) + 1 > MQTT_MAX_PACKET_SIZE) {
    _logger.logln(F("✖ setNodeProperty(): content to send is too long"));
    return false;
  }

  return _mqttClient.publish(value, retained);
}

bool HomieClass::publishRaw(const char* topic, const char* value, bool retained) {
  if (!this->isReadyToOperate()) {
    _logger.logln(F("✖ publishRaw(): impossible now"));
    return false;
  }
  auto topiclen = strlen(topic);
  if (5 + 2 + topiclen + strlen(value) + 1 > MQTT_MAX_PACKET_SIZE) {
    _logger.logln(F("✖ publishRaw(): content to send is too long"));
    return false;
  }
  auto &cli = _mqttClient;
  auto buf = cli.getTopicBuffer();
  memcpy(buf, topic, topiclen + 1);
  return cli.publish(value, retained);
}

HomieClass Homie;

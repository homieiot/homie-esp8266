#include "Homie.hpp"

using namespace HomieInternals;

HomieClass::HomieClass()
: _setupCalled(false)
, __HOMIE_SIGNATURE("\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25") {
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

  _bootNormal.attachInterface(&_interface);
  _bootConfig.attachInterface(&_interface);
}

HomieClass::~HomieClass() {
}

void HomieClass::_checkBeforeSetup(const __FlashStringHelper* functionName) const {
  if (_setupCalled) {
    _logger.log(F("✖ "));
    _logger.log(functionName);
    _logger.logln(F("(): has to be called before setup()"));
    _logger.flush();
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
      default:
        _logger.logln(F("✖ The boot mode is invalid"));
        _logger.flush();
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
    _logger.flush();
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
    _logger.flush();
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
  _config.erase();
}

void HomieClass::setNodeProperty(const HomieNode& node, const char* property, const char* value, uint8_t qos, bool retained) {
  if (!isReadyToOperate()) {
    _logger.logln(F("✖ setNodeProperty(): impossible now"));
    return;
  }

  char* topic = new char[strlen(_interface.config->get().mqtt.baseTopic) + strlen(_interface.config->get().deviceId) + 1 + strlen(node.getId()) + 1 + strlen(property) + 1];
  strcpy(topic, _interface.config->get().mqtt.baseTopic);
  strcat(topic, _interface.config->get().deviceId);
  strcat_P(topic, PSTR("/"));
  strcat(topic, node.getId());
  strcat_P(topic, PSTR("/"));
  strcat(topic, property);

  _mqttClient.publish(topic, qos, retained, value);
  delete[] topic;
}

const ConfigStruct& HomieClass::getConfiguration() const {
  return _config.get();
}

AsyncMqttClient& HomieClass::getMqttClient() {
  return _mqttClient;
}

HomieClass Homie;

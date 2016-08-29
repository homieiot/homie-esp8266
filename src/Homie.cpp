#include "Homie.hpp"

using namespace HomieInternals;

SendingPromise::SendingPromise(HomieClass* homie)
: _homie(homie)
, _node(nullptr)
, _property(nullptr)
, _qos(0)
, _retained(false) {

}

SendingPromise& SendingPromise::setQos(uint8_t qos){
  _qos = qos;
}

SendingPromise& SendingPromise::setRetained(bool retained) {
  _retained = retained;
}

SendingPromise& SendingPromise::setRange(HomieRange range) {
  _range = range;
}

SendingPromise& SendingPromise::setRangeIndex(uint16_t rangeIndex) {
  HomieRange range; range.isRange = true; range.index = rangeIndex; _range = range;
}

void SendingPromise::send(const String& value) {
  if (!_homie->isConnected()) {
    _homie->_logger.logln(F("✖ setNodeProperty(): impossible now"));
    return;
  }

  char* topic = new char[strlen(_homie->getConfiguration().mqtt.baseTopic) + strlen(_homie->getConfiguration().deviceId) + 1 + strlen(_node->getId()) + 1 + strlen(_property->c_str()) + 6 + 1];  // last + 6 for range _65536
  strcpy(topic, _homie->getConfiguration().mqtt.baseTopic);
  strcat(topic, _homie->getConfiguration().deviceId);
  strcat_P(topic, PSTR("/"));
  strcat(topic, _node->getId());
  strcat_P(topic, PSTR("/"));
  strcat(topic, _property->c_str());

  if (_range.isRange) {
    char rangeStr[5 + 1];  // max 65536
    itoa(_range.index, rangeStr, 10);
    strcat_P(topic, PSTR("_"));
    strcat(topic, rangeStr);
  }

  _homie->getMqttClient().publish(topic, _qos, _retained, value.c_str());
  delete[] topic;
}

SendingPromise& SendingPromise::setNode(const HomieNode& node){
  _node = &node;
}

SendingPromise& SendingPromise::setProperty(const String& property) {
  _property = &property;
}

const HomieNode* SendingPromise::getNode() const {
  return _node;
}

const String* SendingPromise::getProperty() const {
  return _property;
}

uint8_t SendingPromise::getQos() const {
  return _qos;
}

HomieRange SendingPromise::getRange() const {
  return _range;
}

bool SendingPromise::isRetained() const {
  return _retained;
}

HomieClass::HomieClass()
: _setupCalled(false)
, _sendingPromise(this)
, __HOMIE_SIGNATURE("\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25") {
  strcpy(_interface.brand, DEFAULT_BRAND);
  _interface.standalone = false;
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
  _interface.globalInputHandler = [](String node, String property, HomieRange range, String value) { return false; };
  _interface.setupFunction = []() {};
  _interface.loopFunction = []() {};
  _interface.eventHandler = [](HomieEvent event) {};
  _interface.connected = false;
  _interface.logger = &_logger;
  _interface.blinker = &_blinker;
  _interface.config = &_config;
  _interface.mqttClient = &_mqttClient;

  Helpers::generateDeviceId();

  _config.attachInterface(&_interface);
  _blinker.attachInterface(&_interface);

  _bootStandalone.attachInterface(&_interface);
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
    if (_interface.standalone && !_config.canBypassStandalone()) {
      _boot = &_bootStandalone;
      _logger.logln(F("Triggering STANDALONE_MODE event..."));
      _interface.eventHandler(HomieEvent::STANDALONE_MODE);
    } else {
      _boot = &_bootConfig;
      _logger.logln(F("Triggering CONFIGURATION_MODE event..."));
      _interface.eventHandler(HomieEvent::CONFIGURATION_MODE);
    }
  } else {
    switch (_config.getBootMode()) {
      case BOOT_NORMAL:
        _boot = &_bootNormal;
        _logger.logln(F("Triggering NORMAL_MODE event..."));
        _interface.eventHandler(HomieEvent::NORMAL_MODE);
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

HomieClass& HomieClass::disableLogging() {
  _checkBeforeSetup(F("disableLogging"));

  _logger.setLogging(false);

  return *this;
}

HomieClass& HomieClass::setLoggingPrinter(Print* printer) {
  _checkBeforeSetup(F("setLoggingPrinter"));

  _logger.setPrinter(printer);

  return *this;
}

HomieClass& HomieClass::disableLedFeedback() {
  _checkBeforeSetup(F("disableLedFeedback"));

  _interface.led.enabled = false;

  return *this;
}

HomieClass& HomieClass::setLedPin(uint8_t pin, uint8_t on) {
  _checkBeforeSetup(F("setLedPin"));

  _interface.led.pin = pin;
  _interface.led.on = on;

  return *this;
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

void HomieClass::setIdle(bool idle) {
  _interface.reset.able = idle;
}

HomieClass& HomieClass::setGlobalInputHandler(GlobalInputHandler inputHandler) {
  _checkBeforeSetup(F("setGlobalInputHandler"));

  _interface.globalInputHandler = inputHandler;

  return *this;
}

HomieClass& HomieClass::setResetFunction(ResetFunction function) {
  _checkBeforeSetup(F("setResetFunction"));

  _interface.reset.userFunction = function;

  return *this;
}

HomieClass& HomieClass::setSetupFunction(OperationFunction function) {
  _checkBeforeSetup(F("setSetupFunction"));

  _interface.setupFunction = function;

  return *this;
}

HomieClass& HomieClass::setLoopFunction(OperationFunction function) {
  _checkBeforeSetup(F("setLoopFunction"));

  _interface.loopFunction = function;

  return *this;
}

HomieClass& HomieClass::setStandalone() {
  _checkBeforeSetup(F("setStandalone"));

  _interface.standalone = true;

  return *this;
}

bool HomieClass::isConfigured() const {
  return _config.getBootMode() == BOOT_NORMAL;
}

bool HomieClass::isConnected() const {
  return _interface.connected;
}

HomieClass& HomieClass::onEvent(EventHandler handler) {
  _checkBeforeSetup(F("onEvent"));

  _interface.eventHandler = handler;

  return *this;
}

HomieClass& HomieClass::setResetTrigger(uint8_t pin, uint8_t state, uint16_t time) {
  _checkBeforeSetup(F("setResetTrigger"));

  _interface.reset.enabled = true;
  _interface.reset.triggerPin = pin;
  _interface.reset.triggerState = state;
  _interface.reset.triggerTime = time;

  return *this;
}

HomieClass& HomieClass::disableResetTrigger() {
  _checkBeforeSetup(F("disableResetTrigger"));

  _interface.reset.enabled = false;

  return *this;
}

void HomieClass::eraseConfiguration() {
  _config.erase();
}

const ConfigStruct& HomieClass::getConfiguration() const {
  return _config.get();
}

AsyncMqttClient& HomieClass::getMqttClient() {
  return _mqttClient;
}

HomieClass Homie;

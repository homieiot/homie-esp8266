#include "Homie.hpp"

using namespace HomieInternals;

SendingPromise::SendingPromise()
: _node(nullptr)
, _property(nullptr)
, _qos(0)
, _retained(false) {
}

SendingPromise& SendingPromise::setQos(uint8_t qos) {
  _qos = qos;
}

SendingPromise& SendingPromise::setRetained(bool retained) {
  _retained = retained;
}

SendingPromise& SendingPromise::setRange(const HomieRange& range) {
  _range = range;
}

SendingPromise& SendingPromise::setRange(uint16_t rangeIndex) {
  HomieRange range;
  range.isRange = true;
  range.index = rangeIndex;
  _range = range;
}

uint16_t SendingPromise::send(const String& value) {
  if (!Interface::get().connected) {
    Interface::get().logger->println(F("✖ setNodeProperty(): impossible now"));
    return 0;
  }

  char* topic = new char[strlen(Interface::get().config->get().mqtt.baseTopic) + strlen(Interface::get().config->get().deviceId) + 1 + strlen(_node->getId()) + 1 + strlen(_property->c_str()) + 6 + 1];  // last + 6 for range _65536
  strcpy(topic, Interface::get().config->get().mqtt.baseTopic);
  strcat(topic, Interface::get().config->get().deviceId);
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

  uint16_t packetId = Interface::get().mqttClient->publish(topic, _qos, _retained, value.c_str());
  delete[] topic;

  return packetId;
}

SendingPromise& SendingPromise::setNode(const HomieNode& node) {
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
, __HOMIE_SIGNATURE("\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25") {
  strcpy(Interface::get().brand, DEFAULT_BRAND);
  Interface::get().standalone = false;
  strcpy(Interface::get().firmware.name, DEFAULT_FW_NAME);
  strcpy(Interface::get().firmware.version, DEFAULT_FW_VERSION);
  Interface::get().led.enabled = true;
  Interface::get().led.pin = BUILTIN_LED;
  Interface::get().led.on = LOW;
  Interface::get().reset.able = true;
  Interface::get().reset.enabled = true;
  Interface::get().reset.triggerPin = DEFAULT_RESET_PIN;
  Interface::get().reset.triggerState = DEFAULT_RESET_STATE;
  Interface::get().reset.triggerTime = DEFAULT_RESET_TIME;
  Interface::get().reset.userFunction = []() { return false; };
  Interface::get().globalInputHandler = [](const HomieNode& node, const String& property, const HomieRange& range, const String& value) { return false; };
  Interface::get().broadcastHandler = [](const String& level, const String& value) { return false; };
  Interface::get().setupFunction = []() {};
  Interface::get().loopFunction = []() {};
  Interface::get().eventHandler = [](const HomieEvent& event) {};
  Interface::get().connected = false;
  Interface::get().mqttClient = &_mqttClient;
  Interface::get().blinker = &_blinker;
  Interface::get().logger = &_logger;
  Interface::get().config = &_config;

  DeviceId::generate();
}

HomieClass::~HomieClass() {
}

void HomieClass::_checkBeforeSetup(const __FlashStringHelper* functionName) {
  if (_setupCalled) {
    Interface::get().logger->print(F("✖ "));
    Interface::get().logger->print(functionName);
    Interface::get().logger->println(F("(): has to be called before setup()"));
    Serial.flush();
    abort();
  }
}

void HomieClass::setup() {
  _setupCalled = true;

  if (!Interface::get().config->load()) {
    if (Interface::get().standalone && !Interface::get().config->canBypassStandalone()) {
      _boot = &_bootStandalone;
      Interface::get().logger->println(F("Triggering STANDALONE_MODE event..."));
      Interface::get().event.type = HomieEventType::STANDALONE_MODE;
      Interface::get().eventHandler(Interface::get().event);
    } else {
      _boot = &_bootConfig;
      Interface::get().logger->println(F("Triggering CONFIGURATION_MODE event..."));
      Interface::get().event.type = HomieEventType::CONFIGURATION_MODE;
      Interface::get().eventHandler(Interface::get().event);
    }
  } else {
    switch (Interface::get().config->getBootMode()) {
      case BOOT_NORMAL:
        _boot = &_bootNormal;
        Interface::get().logger->println(F("Triggering NORMAL_MODE event..."));
        Interface::get().event.type = HomieEventType::NORMAL_MODE;
        Interface::get().eventHandler(Interface::get().event);
        break;
      default:
        Interface::get().logger->println(F("✖ The boot mode is invalid"));
        Serial.flush();
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

  Interface::get().logger->setLogging(false);

  return *this;
}

HomieClass& HomieClass::setLoggingPrinter(Print* printer) {
  _checkBeforeSetup(F("setLoggingPrinter"));

  Interface::get().logger->setPrinter(printer);

  return *this;
}

HomieClass& HomieClass::disableLedFeedback() {
  _checkBeforeSetup(F("disableLedFeedback"));

  Interface::get().led.enabled = false;

  return *this;
}

HomieClass& HomieClass::setLedPin(uint8_t pin, uint8_t on) {
  _checkBeforeSetup(F("setLedPin"));

  Interface::get().led.pin = pin;
  Interface::get().led.on = on;

  return *this;
}

void HomieClass::__setFirmware(const char* name, const char* version) {
  _checkBeforeSetup(F("setFirmware"));
  if (strlen(name) + 1 - 10 > MAX_FIRMWARE_NAME_LENGTH || strlen(version) + 1 - 10 > MAX_FIRMWARE_VERSION_LENGTH) {
    Interface::get().logger->println(F("✖ setFirmware(): either the name or version string is too long"));
    Serial.flush();
    abort();
  }

  strncpy(Interface::get().firmware.name, name + 5, strlen(name) - 10);
  Interface::get().firmware.name[strlen(name) - 10] = '\0';
  strncpy(Interface::get().firmware.version, version + 5, strlen(version) - 10);
  Interface::get().firmware.version[strlen(version) - 10] = '\0';
}

void HomieClass::__setBrand(const char* brand) {
  _checkBeforeSetup(F("setBrand"));
  if (strlen(brand) + 1 - 10 > MAX_BRAND_LENGTH) {
    Interface::get().logger->println(F("✖ setBrand(): the brand string is too long"));
    Serial.flush();
    abort();
  }

  strncpy(Interface::get().brand, brand + 5, strlen(brand) - 10);
  Interface::get().brand[strlen(brand) - 10] = '\0';
}

void HomieClass::setIdle(bool idle) {
  Interface::get().reset.able = idle;
}

HomieClass& HomieClass::setGlobalInputHandler(GlobalInputHandler inputHandler) {
  _checkBeforeSetup(F("setGlobalInputHandler"));

  Interface::get().globalInputHandler = inputHandler;

  return *this;
}

HomieClass& HomieClass::setBroadcastHandler(BroadcastHandler broadcastHandler) {
  _checkBeforeSetup(F("setBroadcastHandler"));

  Interface::get().broadcastHandler = broadcastHandler;

  return *this;
}

HomieClass& HomieClass::setResetFunction(ResetFunction function) {
  _checkBeforeSetup(F("setResetFunction"));

  Interface::get().reset.userFunction = function;

  return *this;
}

HomieClass& HomieClass::setSetupFunction(OperationFunction function) {
  _checkBeforeSetup(F("setSetupFunction"));

  Interface::get().setupFunction = function;

  return *this;
}

HomieClass& HomieClass::setLoopFunction(OperationFunction function) {
  _checkBeforeSetup(F("setLoopFunction"));

  Interface::get().loopFunction = function;

  return *this;
}

HomieClass& HomieClass::setStandalone() {
  _checkBeforeSetup(F("setStandalone"));

  Interface::get().standalone = true;

  return *this;
}

bool HomieClass::isConfigured() const {
  return Interface::get().config->getBootMode() == BOOT_NORMAL;
}

bool HomieClass::isConnected() const {
  return Interface::get().connected;
}

HomieClass& HomieClass::onEvent(EventHandler handler) {
  _checkBeforeSetup(F("onEvent"));

  Interface::get().eventHandler = handler;

  return *this;
}

HomieClass& HomieClass::setResetTrigger(uint8_t pin, uint8_t state, uint16_t time) {
  _checkBeforeSetup(F("setResetTrigger"));

  Interface::get().reset.enabled = true;
  Interface::get().reset.triggerPin = pin;
  Interface::get().reset.triggerState = state;
  Interface::get().reset.triggerTime = time;

  return *this;
}

HomieClass& HomieClass::disableResetTrigger() {
  _checkBeforeSetup(F("disableResetTrigger"));

  Interface::get().reset.enabled = false;

  return *this;
}

void HomieClass::eraseConfiguration() {
  Interface::get().config->erase();
}

const ConfigStruct& HomieClass::getConfiguration() const {
  return Interface::get().config->get();
}

AsyncMqttClient& HomieClass::getMqttClient() {
  return _mqttClient;
}

void HomieClass::prepareToSleep() {
  _boot->prepareToSleep();
}

HomieClass Homie;

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
    Interface::get().getLogger() << F("✖ setNodeProperty(): impossible now") << endl;
    return 0;
  }

  char* topic = new char[strlen(Interface::get().getConfig().get().mqtt.baseTopic) + strlen(Interface::get().getConfig().get().deviceId) + 1 + strlen(_node->getId()) + 1 + strlen(_property->c_str()) + 6 + 1];  // last + 6 for range _65536
  strcpy(topic, Interface::get().getConfig().get().mqtt.baseTopic);
  strcat(topic, Interface::get().getConfig().get().deviceId);
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

  uint16_t packetId = Interface::get().getMqttClient().publish(topic, _qos, _retained, value.c_str());
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
, _firmwareSet(false)
, __HOMIE_SIGNATURE("\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25") {
  strcpy(Interface::get().brand, DEFAULT_BRAND);
  Interface::get().standalone = false;
  Interface::get().led.enabled = true;
  Interface::get().led.pin = BUILTIN_LED;
  Interface::get().led.on = LOW;
  Interface::get().reset.idle = true;
  Interface::get().reset.enabled = true;
  Interface::get().reset.triggerPin = DEFAULT_RESET_PIN;
  Interface::get().reset.triggerState = DEFAULT_RESET_STATE;
  Interface::get().reset.triggerTime = DEFAULT_RESET_TIME;
  Interface::get().reset.flaggedBySketch = false;
  Interface::get().globalInputHandler = [](const HomieNode& node, const String& property, const HomieRange& range, const String& value) { return false; };
  Interface::get().broadcastHandler = [](const String& level, const String& value) { return false; };
  Interface::get().setupFunction = []() {};
  Interface::get().loopFunction = []() {};
  Interface::get().eventHandler = [](const HomieEvent& event) {};
  Interface::get().connected = false;
  Interface::get()._mqttClient = &_mqttClient;
  Interface::get()._blinker = &_blinker;
  Interface::get()._logger = &_logger;
  Interface::get()._config = &_config;

  DeviceId::generate();
}

HomieClass::~HomieClass() {
}

void HomieClass::_checkBeforeSetup(const __FlashStringHelper* functionName) {
  if (_setupCalled) {
    Interface::get().getLogger() << F("✖ ") << functionName << F("(): has to be called before setup()") << endl;
    Serial.flush();
    abort();
  }
}

void HomieClass::setup() {
  _setupCalled = true;

  if (!_firmwareSet) {
    Interface::get().getLogger() << F("✖ firmware must be set before calling setup()") << endl;
    Serial.flush();
    abort();
  }

  if (!Interface::get().getConfig().load()) {
    if (Interface::get().standalone && !Interface::get().getConfig().canBypassStandalone()) {
      _boot = &_bootStandalone;
      Interface::get().getLogger() << F("Triggering STANDALONE_MODE event...") << endl;
      Interface::get().event.type = HomieEventType::STANDALONE_MODE;
      Interface::get().eventHandler(Interface::get().event);
    } else {
      _boot = &_bootConfig;
      Interface::get().getLogger() << F("Triggering CONFIGURATION_MODE event...") << endl;
      Interface::get().event.type = HomieEventType::CONFIGURATION_MODE;
      Interface::get().eventHandler(Interface::get().event);
    }
  } else {
    switch (Interface::get().getConfig().getBootMode()) {
      case BootMode::NORMAL:
        _boot = &_bootNormal;
        Interface::get().getLogger() << F("Triggering NORMAL_MODE event...") << endl;
        Interface::get().event.type = HomieEventType::NORMAL_MODE;
        Interface::get().eventHandler(Interface::get().event);
        break;
      default:
        Interface::get().getLogger() << F("✖ The boot mode is invalid") << endl;
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

  Interface::get().getLogger().setLogging(false);

  return *this;
}

HomieClass& HomieClass::setLoggingPrinter(Print* printer) {
  _checkBeforeSetup(F("setLoggingPrinter"));

  Interface::get().getLogger().setPrinter(printer);

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
    Interface::get().getLogger() << F("✖ setFirmware(): either the name or version string is too long") << endl;
    Serial.flush();
    abort();
  }

  strncpy(Interface::get().firmware.name, name + 5, strlen(name) - 10);
  Interface::get().firmware.name[strlen(name) - 10] = '\0';
  strncpy(Interface::get().firmware.version, version + 5, strlen(version) - 10);
  Interface::get().firmware.version[strlen(version) - 10] = '\0';
  _firmwareSet = true;
}

void HomieClass::__setBrand(const char* brand) {
  _checkBeforeSetup(F("setBrand"));
  if (strlen(brand) + 1 - 10 > MAX_BRAND_LENGTH) {
    Interface::get().getLogger() << F("✖ setBrand(): the brand string is too long") << endl;
    Serial.flush();
    abort();
  }

  strncpy(Interface::get().brand, brand + 5, strlen(brand) - 10);
  Interface::get().brand[strlen(brand) - 10] = '\0';
}

void HomieClass::reset() {
  Interface::get().reset.flaggedBySketch = true;
}

void HomieClass::setIdle(bool idle) {
  Interface::get().reset.idle = idle;
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
  return Interface::get().getConfig().getBootMode() == BootMode::NORMAL;
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

const ConfigStruct& HomieClass::getConfiguration() const {
  return Interface::get().getConfig().get();
}

AsyncMqttClient& HomieClass::getMqttClient() {
  return _mqttClient;
}

void HomieClass::prepareToSleep() {
  _boot->prepareToSleep();
}

HomieClass Homie;

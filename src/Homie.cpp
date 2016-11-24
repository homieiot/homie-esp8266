#include "Homie.hpp"

using namespace HomieInternals;

HomieClass::HomieClass()
: _setupCalled(false)
, _firmwareSet(false)
, __HOMIE_SIGNATURE("\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25") {
  strcpy(Interface::get().brand, DEFAULT_BRAND);
  Interface::get().bootMode = BootMode::UNDEFINED;
  Interface::get().led.enabled = true;
  Interface::get().led.pin = BUILTIN_LED;
  Interface::get().led.on = LOW;
  Interface::get().reset.idle = true;
  Interface::get().reset.enabled = true;
  Interface::get().reset.triggerPin = DEFAULT_RESET_PIN;
  Interface::get().reset.triggerState = DEFAULT_RESET_STATE;
  Interface::get().reset.triggerTime = DEFAULT_RESET_TIME;
  Interface::get().reset.flaggedBySketch = false;
  Interface::get().flaggedForSleep = false;
  Interface::get().globalInputHandler = [](const HomieNode& node, const String& property, const HomieRange& range, const String& value) { return false; };
  Interface::get().broadcastHandler = [](const String& level, const String& value) { return false; };
  Interface::get().setupFunction = []() {};
  Interface::get().loopFunction = []() {};
  Interface::get().eventHandler = [](const HomieEvent& event) {};
  Interface::get().connected = false;
  Interface::get()._mqttClient = &_mqttClient;
  Interface::get()._sendingPromise = &_sendingPromise;
  Interface::get()._blinker = &_blinker;
  Interface::get()._logger = &_logger;
  Interface::get()._config = &_config;

  DeviceId::generate();
}

HomieClass::~HomieClass() {
}

void HomieClass::_checkBeforeSetup(const __FlashStringHelper* functionName) const {
  if (_setupCalled) {
    Interface::get().getLogger() << F("✖ ") << functionName << F("(): has to be called before setup()") << endl;
    Serial.flush();
    abort();
  }
}

void HomieClass::setup() {
  _setupCalled = true;

  if (!_firmwareSet) {
    Interface::get().getLogger() << F("✖ Firmware name must be set before calling setup()") << endl;
    Serial.flush();
    abort();
  }

  // boot mode set during this boot by application before Homie.setup()
  BootMode _applicationBootMode = Interface::get().bootMode;

  // boot mode set before resetting the device. If application has defined a boot mode, this will be ignored
  BootMode _nextBootMode = Interface::get().getConfig().getBootModeOnNextBoot();
  if (_nextBootMode != BootMode::UNDEFINED) {
    Interface::get().getConfig().setBootModeOnNextBoot(BootMode::UNDEFINED);
  }

  BootMode _selectedBootMode = BootMode::CONFIG;

  // select boot mode source
  if (_applicationBootMode != BootMode::UNDEFINED) {
    _selectedBootMode = _applicationBootMode;
  } else if (_nextBootMode != BootMode::UNDEFINED) {
    _selectedBootMode = _nextBootMode;
  } else {
    _selectedBootMode = BootMode::NORMAL;
  }

  // validate selected mode and fallback as needed
  if (_selectedBootMode == BootMode::NORMAL && !Interface::get().getConfig().load()) {
    Interface::get().getLogger() << F("Configuration invalid. Using CONFIG MODE") << endl;
    _selectedBootMode = BootMode::CONFIG;
  }

  // run selected mode
  if (_selectedBootMode == BootMode::NORMAL) {
    _boot = &_bootNormal;
    Interface::get().event.type = HomieEventType::NORMAL_MODE;
    Interface::get().eventHandler(Interface::get().event);

  } else if (_selectedBootMode == BootMode::CONFIG) {
    _boot = &_bootConfig;
    Interface::get().event.type = HomieEventType::CONFIGURATION_MODE;
    Interface::get().eventHandler(Interface::get().event);

  } else if (_selectedBootMode == BootMode::STANDALONE) {
    _boot = &_bootStandalone;
    Interface::get().event.type = HomieEventType::STANDALONE_MODE;
    Interface::get().eventHandler(Interface::get().event);

  } else {
    Interface::get().getLogger() << F("✖ Boot mode invalid") << endl;
    Serial.flush();
    abort();
  }

  _boot->setup();
}

void HomieClass::loop() {
  _boot->loop();

  if (_flaggedForReboot && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;
    Interface::get().getLogger() << F("Triggering ABOUT_TO_RESET event...") << endl;
    Interface::get().event.type = HomieEventType::ABOUT_TO_RESET;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().getLogger() << F("↻ Rebooting device...") << endl;
    Serial.flush();
    ESP.restart();
  }
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

void HomieClass::__setBrand(const char* brand) const {
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

void HomieClass::reboot() {
  _flaggedForReboot = true;
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

HomieClass& HomieClass::setBootMode(BootMode bootMode) {
  _checkBeforeSetup(F("setBootMode"));
  Interface::get().bootMode = bootMode;
  return *this;
}

HomieClass& HomieClass::setBootModeNextBoot(BootMode bootMode) {
  Interface::get().getConfig().setBootModeOnNextBoot(bootMode);
  return *this;
}

bool HomieClass::isConfigured() const {
  return Interface::get().getConfig().load();
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
  if (Interface::get().connected) {
    Interface::get().flaggedForSleep = true;
  } else {
    Interface::get().getLogger() << F("Triggering READY_TO_SLEEP event...") << endl;
    Interface::get().event.type = HomieEventType::READY_TO_SLEEP;
    Interface::get().eventHandler(Interface::get().event);
  }
}

HomieClass Homie;

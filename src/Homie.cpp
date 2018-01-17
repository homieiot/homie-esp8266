#include "Homie.hpp"

using namespace HomieInternals;

HomieClass::HomieClass()
  : _setupCalled(false)
  , _firmwareSet(false)
  , _loadedSettings(false)
  , __HOMIE_SIGNATURE("\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25") {
  strlcpy(Interface::get().brand, DEFAULT_BRAND, MAX_BRAND_STRING_LENGTH);
  Interface::get().bootMode = HomieBootMode::UNDEFINED;
  Interface::get().configurationAp.secured = false;
  Interface::get().led.enabled = true;
  Interface::get().led.pin = BUILTIN_LED;
  Interface::get().led.on = LOW;
  Interface::get().reset.enabled = true;
  Interface::get().reset.triggerPin = DEFAULT_RESET_PIN;
  Interface::get().reset.triggerState = DEFAULT_RESET_STATE;
  Interface::get().reset.triggerTime = DEFAULT_RESET_TIME;
  Interface::get().flags.idle = true;
  Interface::get().flags.disable = false;
  Interface::get().flags.reboot = false;
  Interface::get().flags.reset = false;
  Interface::get().flags.sleep = false;
  Interface::get().globalInputHandler = [](const HomieNode& node, const String& property, const HomieRange& range, const String& value) { return false; };
  Interface::get().broadcastHandler = [](const String& level, const String& value) { return false; };
  Interface::get().setupFunction = []() {};
  Interface::get().loopFunction = []() {};
  Interface::get().eventHandler = [](const HomieEvent& event) {};
  Interface::get().ready = false;
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
    String message;
    message.concat(F("✖ "));
    message.concat(functionName);
    message.concat(F("(): has to be called before setup()"));
    Helpers::abort(message);
  }
}

void HomieClass::setup() {
  _setupCalled = true;

  // check if firmware is set
  if (!_firmwareSet) {
    Helpers::abort(F("✖ Firmware name must be set before calling setup()"));
    return;  // never reached, here for clarity
  }

  // load settings & load config
  if (!_loadedSettings) loadSettings();

  // boot mode set during this boot by application before Homie.setup()
  HomieBootMode _applicationHomieBootMode = Interface::get().bootMode;

  // boot mode set before resetting the device. If application has defined a boot mode, this will be ignored
  HomieBootMode _nextHomieBootMode = Interface::get().getConfig().getHomieBootModeOnNextBoot();
  if (_nextHomieBootMode != HomieBootMode::UNDEFINED) {
    Interface::get().getConfig().setHomieBootModeOnNextBoot(HomieBootMode::UNDEFINED);
  }

  HomieBootMode _selectedHomieBootMode = HomieBootMode::CONFIGURATION;

  // select boot mode source
  if (_applicationHomieBootMode != HomieBootMode::UNDEFINED) {
    _selectedHomieBootMode = _applicationHomieBootMode;
  } else if (_nextHomieBootMode != HomieBootMode::UNDEFINED) {
    _selectedHomieBootMode = _nextHomieBootMode;
  } else {
    _selectedHomieBootMode = HomieBootMode::NORMAL;
  }

  // validate selected mode and fallback as needed
  if (_selectedHomieBootMode == HomieBootMode::NORMAL && !Interface::get().getConfig().isValid()) {
    Interface::get().getLogger() << F("Configuration invalid. Using CONFIG MODE") << endl;
    _selectedHomieBootMode = HomieBootMode::CONFIGURATION;
  }

  // run selected mode
  if (_selectedHomieBootMode == HomieBootMode::NORMAL) {
    _boot = &_bootNormal;
    Interface::get().event.type = HomieEventType::NORMAL_MODE;
    Interface::get().eventHandler(Interface::get().event);
  } else if (_selectedHomieBootMode == HomieBootMode::CONFIGURATION) {
    _boot = &_bootConfig;
    Interface::get().event.type = HomieEventType::CONFIGURATION_MODE;
    Interface::get().eventHandler(Interface::get().event);
  } else if (_selectedHomieBootMode == HomieBootMode::STANDALONE) {
    _boot = &_bootStandalone;
    Interface::get().event.type = HomieEventType::STANDALONE_MODE;
    Interface::get().eventHandler(Interface::get().event);
  } else {
    Helpers::abort(F("✖ Boot mode invalid"));
    return;  // never reached, here for clarity
  }

  // Set the boot mode
  Interface::get().bootMode = _selectedHomieBootMode;

  _boot->setup();
}

void HomieClass::loop() {
  _boot->loop();
}

bool HomieClass::loadSettings() {
  // Check the max allowed setting elements
  if (IHomieSetting::settings.size() > MAX_CONFIG_SETTING_SIZE) {
    Helpers::abort(F("✖ Settings exceed set limit of elelement."));
    return false;  // never reached, here for clarity
  }

  // Check if default settings values are valid
  for (IHomieSetting& iSetting : IHomieSetting::settings) {
    bool defaultSettingsValuesValid = true;
    if (iSetting.isBool()) {
      HomieSetting<bool>& setting = static_cast<HomieSetting<bool>&>(iSetting);
      if (!setting.isRequired() && !setting._validate(setting.get())) {
        defaultSettingsValuesValid = false;
      }
    } else if (iSetting.isLong()) {
      HomieSetting<long>& setting = static_cast<HomieSetting<long>&>(iSetting);
      if (!setting.isRequired() && !setting._validate(setting.get())) {
        defaultSettingsValuesValid = false;
      }
    } else if (iSetting.isDouble()) {
      HomieSetting<double>& setting = static_cast<HomieSetting<double>&>(iSetting);
      if (!setting.isRequired() && !setting._validate(setting.get())) {
        defaultSettingsValuesValid = false;
      }
    } else if (iSetting.isConstChar()) {
      HomieSetting<const char*>& setting = static_cast<HomieSetting<const char*>&>(iSetting);
      if (!setting.isRequired() && !setting._validate(setting.get())) {
        defaultSettingsValuesValid = false;
      }
    }
    if (!defaultSettingsValuesValid) {
      String error = F("✖ Default setting value does not pass validator test for ");
      error.concat(iSetting.getName());
      error.concat(F(" setting"));
      Helpers::abort(error);
      return false;  // never reached, here for clarity
    }
  }

  _loadedSettings = true;

  // load the config to get the config for settings;
  return Interface::get().getConfig().load();
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

HomieClass& HomieClass::setConfigurationApPassword(const char* password) {
  _checkBeforeSetup(F("setConfigurationApPassword"));

  Interface::get().configurationAp.secured = true;
  strlcpy(Interface::get().configurationAp.password, password, MAX_WIFI_PASSWORD_STRING_LENGTH);
  return *this;
}

void HomieClass::__setFirmware(const char* name, const char* version) {
  _checkBeforeSetup(F("setFirmware"));
  if (strlen(name) + 1 > MAX_FIRMWARE_NAME_STRING_LENGTH || strlen(version) + 1 > MAX_FIRMWARE_VERSION_STRING_LENGTH) {
    Helpers::abort(F("✖ setFirmware(): either the name or version string is too long"));
    return;  // never reached, here for clarity
  }

  strncpy(Interface::get().firmware.name, name, MAX_FIRMWARE_NAME_STRING_LENGTH);
  Interface::get().firmware.name[strlen(name)] = '\0';
  strncpy(Interface::get().firmware.version, version, MAX_FIRMWARE_VERSION_STRING_LENGTH);
  Interface::get().firmware.version[strlen(version)] = '\0';
  _firmwareSet = true;
}

void HomieClass::__setBrand(const char* brand) const {
  _checkBeforeSetup(F("setBrand"));
  if (strlen(brand) + 1 > MAX_BRAND_STRING_LENGTH) {
    Helpers::abort(F("✖ setBrand(): the brand string is too long"));
    return;  // never reached, here for clarity
  }

  strncpy(Interface::get().brand, brand, MAX_BRAND_STRING_LENGTH);
  Interface::get().brand[strlen(brand)] = '\0';
}

void HomieClass::reset() {
  Interface::get().getLogger() << F("Flagged for reset by sketch") << endl;
  Interface::get().flags.disable = true;
  Interface::get().flags.reset = true;
}

void HomieClass::reboot() {
  Interface::get().getLogger() << F("Flagged for reboot by sketch") << endl;
  Interface::get().flags.disable = true;
  Interface::get().flags.reboot = true;
}

void HomieClass::setIdle(bool idle) {
  Interface::get().flags.idle = idle;
}

HomieClass& HomieClass::setGlobalInputHandler(const GlobalInputHandler& globalInputHandler) {
  _checkBeforeSetup(F("setGlobalInputHandler"));

  Interface::get().globalInputHandler = globalInputHandler;

  return *this;
}

HomieClass& HomieClass::setBroadcastHandler(const BroadcastHandler& broadcastHandler) {
  _checkBeforeSetup(F("setBroadcastHandler"));

  Interface::get().broadcastHandler = broadcastHandler;

  return *this;
}

HomieClass& HomieClass::setSetupFunction(const CallbackFunction& function) {
  _checkBeforeSetup(F("setSetupFunction"));

  Interface::get().setupFunction = function;

  return *this;
}

HomieClass& HomieClass::setLoopFunction(const CallbackFunction& function) {
  _checkBeforeSetup(F("setLoopFunction"));

  Interface::get().loopFunction = function;

  return *this;
}

HomieClass& HomieClass::setHomieBootMode(HomieBootMode bootMode) {
  _checkBeforeSetup(F("setHomieBootMode"));

  Interface::get().bootMode = bootMode;
  return *this;
}

HomieClass& HomieClass::setHomieBootModeOnNextBoot(HomieBootMode bootMode) {
  Interface::get().getConfig().setHomieBootModeOnNextBoot(bootMode);
  return *this;
}

bool HomieClass::isConfigured() {
  return Interface::get().getConfig().isValid();
}

bool HomieClass::isConnected() {
  return Interface::get().ready;
}

HomieClass& HomieClass::onEvent(const EventHandler& handler) {
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

const ConfigStruct& HomieClass::getConfiguration() {
  return Interface::get().getConfig().get();
}

AsyncMqttClient& HomieClass::getMqttClient() {
  return _mqttClient;
}

Logger& HomieClass::getLogger() {
  return _logger;
}

String HomieInternals::HomieClass::getDeviceID() {
  return String(Interface::get().getConfig().get().deviceId);
}

void HomieClass::prepareToSleep() {
  Interface::get().getLogger() << F("Flagged for sleep by sketch") << endl;
  if (Interface::get().ready) {
    Interface::get().flags.disable = true;
    Interface::get().flags.sleep = true;
  } else {
    Interface::get().flags.disable = true;
    Interface::get().getLogger() << F("Triggering READY_TO_SLEEP event...") << endl;
    Interface::get().event.type = HomieEventType::READY_TO_SLEEP;
    Interface::get().eventHandler(Interface::get().event);
  }
}

void HomieClass::doDeepSleep(uint32_t time_us, RFMode mode) {
  Interface::get().getLogger() << F("💤 Device is deep sleeping...") << endl;
  Serial.flush();
  ESP.deepSleep(time_us, mode);
}

HomieClass Homie;

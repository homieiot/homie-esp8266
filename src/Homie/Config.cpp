#include "Config.hpp"

using namespace HomieInternals;

Config::Config()
: _valid(false)
, _configStruct()
, _spiffsBegan(false) {
}

bool Config::_spiffsBegin() {
  if (!_spiffsBegan) {
    _spiffsBegan = SPIFFS.begin();
    if (!_spiffsBegan) Interface::get().getLogger() << F("✖ Cannot mount filesystem") << endl;
  }

  return _spiffsBegan;
}

bool Config::load() {
  if (!_spiffsBegin()) { return false; }

  _valid = false;

  if (!SPIFFS.exists(CONFIG_FILE_PATH)) {
    Interface::get().getLogger() << F("✖ ") << CONFIG_FILE_PATH << F(" doesn't exist") << endl;
    return false;
  }

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  if (!configFile) {
    Interface::get().getLogger() << F("✖ Cannot open config file") << endl;
    return false;
  }

  size_t configSize = configFile.size();

  if (configSize > MAX_JSON_CONFIG_FILE_SIZE) {
    Interface::get().getLogger() << F("✖ Config file too big") << endl;
    return false;
  }

  char buf[MAX_JSON_CONFIG_FILE_SIZE];
  configFile.readBytes(buf, configSize);
  configFile.close();
  buf[configSize] = '\0';

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& parsedJson = jsonBuffer.parseObject(buf);
  if (!parsedJson.success()) {
    Interface::get().getLogger() << F("✖ Invalid JSON in the config file") << endl;
    return false;
  }

  ConfigValidationResult configValidationResult = Validation::validateConfig(parsedJson);
  if (!configValidationResult.valid) {
    Interface::get().getLogger() << F("✖ Config file is not valid, reason: ") << configValidationResult.reason << endl;
    return false;
  }

  const char* reqName = parsedJson["name"];
  const char* reqWifiSsid = parsedJson["wifi"]["ssid"];
  const char* reqWifiPassword = parsedJson["wifi"]["password"];

  const char* reqMqttHost = parsedJson["mqtt"]["host"];
  const char* reqDeviceId = DeviceId::get();
  if (parsedJson.containsKey("device_id")) {
    reqDeviceId = parsedJson["device_id"];
  }
  uint16_t reqMqttPort = DEFAULT_MQTT_PORT;
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("port")) {
    reqMqttPort = parsedJson["mqtt"]["port"];
  }
  const char* reqMqttBaseTopic = DEFAULT_MQTT_BASE_TOPIC;
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("base_topic")) {
    reqMqttBaseTopic = parsedJson["mqtt"]["base_topic"];
  }
  bool reqMqttAuth = false;
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("auth")) {
    reqMqttAuth = parsedJson["mqtt"]["auth"];
  }
  const char* reqMqttUsername = "";
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("username")) {
    reqMqttUsername = parsedJson["mqtt"]["username"];
  }
  const char* reqMqttPassword = "";
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("password")) {
    reqMqttPassword = parsedJson["mqtt"]["password"];
  }

  bool reqOtaEnabled = false;
  if (parsedJson["ota"].as<JsonObject&>().containsKey("enabled")) {
    reqOtaEnabled = parsedJson["ota"]["enabled"];
  }

  strcpy(_configStruct.name, reqName);
  strcpy(_configStruct.wifi.ssid, reqWifiSsid);
  if (reqWifiPassword) strcpy(_configStruct.wifi.password, reqWifiPassword);
  strcpy(_configStruct.deviceId, reqDeviceId);
  strcpy(_configStruct.mqtt.server.host, reqMqttHost);
  _configStruct.mqtt.server.port = reqMqttPort;
  strcpy(_configStruct.mqtt.baseTopic, reqMqttBaseTopic);
  _configStruct.mqtt.auth = reqMqttAuth;
  strcpy(_configStruct.mqtt.username, reqMqttUsername);
  strcpy(_configStruct.mqtt.password, reqMqttPassword);
  _configStruct.ota.enabled = reqOtaEnabled;

  /* Parse the settings */

  JsonObject& settingsObject = parsedJson["settings"].as<JsonObject&>();

  for (IHomieSetting* iSetting : IHomieSetting::settings) {
    if (iSetting->isBool()) {
      HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        setting->set(settingsObject[setting->getName()].as<bool>());
      }
    } else if (iSetting->isLong()) {
      HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        setting->set(settingsObject[setting->getName()].as<long>());
      }
    } else if (iSetting->isDouble()) {
      HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        setting->set(settingsObject[setting->getName()].as<double>());
      }
    } else if (iSetting->isConstChar()) {
      HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        setting->set(strdup(settingsObject[setting->getName()].as<const char*>()));
      }
    }
  }

  _valid = true;
  return true;
}

char* Config::getSafeConfigFile() const {
  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  size_t configSize = configFile.size();

  char buf[MAX_JSON_CONFIG_FILE_SIZE];
  configFile.readBytes(buf, configSize);
  configFile.close();
  buf[configSize] = '\0';

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& parsedJson = jsonBuffer.parseObject(buf);
  parsedJson["wifi"].as<JsonObject&>().remove("password");
  parsedJson["mqtt"].as<JsonObject&>().remove("username");
  parsedJson["mqtt"].as<JsonObject&>().remove("password");

  size_t jsonBufferLength = parsedJson.measureLength() + 1;
  std::unique_ptr<char[]> jsonString(new char[jsonBufferLength]);
  parsedJson.printTo(jsonString.get(), jsonBufferLength);

  return strdup(jsonString.get());
}

void Config::erase() {
  if (!_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);
  SPIFFS.remove(CONFIG_NEXT_BOOT_MODE_FILE_PATH);
}

void Config::setBootModeOnNextBoot(BootMode bootMode) {
  if (!_spiffsBegin()) { return; }

  if (bootMode == BootMode::UNDEFINED) {
    SPIFFS.remove(CONFIG_NEXT_BOOT_MODE_FILE_PATH);
  } else {
    File bootModeFile = SPIFFS.open(CONFIG_NEXT_BOOT_MODE_FILE_PATH, "w");
    if (!bootModeFile) {
      Interface::get().getLogger() << F("✖ Cannot open BOOTMODE file") << endl;
      return;
    }

    bootModeFile.printf("#%d", bootMode);
    bootModeFile.close();
    Interface::get().getLogger().printf("Setting next boot mode to %d\n", bootMode);
  }
}

BootMode Config::getBootModeOnNextBoot() {
  if (!_spiffsBegin()) { return BootMode::UNDEFINED; }

  File bootModeFile = SPIFFS.open(CONFIG_NEXT_BOOT_MODE_FILE_PATH, "r");
  if (bootModeFile) {
    int v = bootModeFile.parseInt();
    bootModeFile.close();
    if (v == 1) {
      return BootMode::STANDALONE;
    } else if (v == 2) {
      return BootMode::CONFIG;
    } else if (v == 3) {
      return BootMode::NORMAL;
    } else {
      return BootMode::UNDEFINED;
    }
  } else {
    return BootMode::UNDEFINED;
  }
}

void Config::write(const JsonObject& config) {
  if (!_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    Interface::get().getLogger() << F("✖ Cannot open config file") << endl;
    return;
  }

  config.printTo(configFile);
  configFile.close();
}

bool Config::patch(const char* patch) {
    if (!_spiffsBegin()) { return false; }

    StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> patchJsonBuffer;
    JsonObject& patchObject = patchJsonBuffer.parseObject(patch);

    if (!patchObject.success()) {
      Interface::get().getLogger() << F("✖ Invalid or too big JSON") << endl;
      return false;
    }

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    if (!configFile) {
      Interface::get().getLogger() << F("✖ Cannot open config file") << endl;
      return false;
    }

    size_t configSize = configFile.size();

    char configJson[MAX_JSON_CONFIG_FILE_SIZE];
    configFile.readBytes(configJson, configSize);
    configFile.close();
    configJson[configSize] = '\0';

    StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> configJsonBuffer;
    JsonObject& configObject = configJsonBuffer.parseObject(configJson);

    for (JsonObject::iterator it = patchObject.begin(); it != patchObject.end(); ++it) {
      if (patchObject[it->key].is<JsonObject&>()) {
        JsonObject& subObject = patchObject[it->key].as<JsonObject&>();
        for (JsonObject::iterator it2 = subObject.begin(); it2 != subObject.end(); ++it2) {
          if (!configObject.containsKey(it->key) || !configObject[it->key].is<JsonObject&>()) {
            String error = "✖ Config does not contain a ";
            error.concat(it->key);
            error.concat(" object");
            Interface::get().getLogger() << error << endl;
            return false;
          }
          JsonObject& subConfigObject = configObject[it->key].as<JsonObject&>();
          subConfigObject[it2->key] = it2->value;
        }
      } else {
        configObject[it->key] = it->value;
      }
    }

    ConfigValidationResult configValidationResult = Validation::validateConfig(configObject);
    if (!configValidationResult.valid) {
      Interface::get().getLogger() << F("✖ Config file is not valid, reason: ") << configValidationResult.reason << endl;
      return false;
    }

    write(configObject);

    return true;
}

bool Config::isValid() const {
  return this->_valid;
}

void Config::log() const {
  Interface::get().getLogger() << F("{} Stored configuration") << endl;
  Interface::get().getLogger() << F("  • Hardware device ID: ") << DeviceId::get() << endl;
  Interface::get().getLogger() << F("  • Device ID: ") << _configStruct.deviceId << endl;
  Interface::get().getLogger() << F("  • Boot mode: ");
  switch (_bootMode) {
    case BootMode::CONFIG:
      Interface::get().getLogger() << F("configuration") << endl;
      break;
    case BootMode::NORMAL:
      Interface::get().getLogger() << F("normal") << endl;
      break;
    case BootMode::STANDALONE:
      Interface::get().getLogger() << F("standalone") << endl;
      break;
    default:
      Interface::get().getLogger() << F("unknown") << endl;
      break;
  }

  Interface::get().getLogger() << F("  • Name: ") << _configStruct.name << endl;

  Interface::get().getLogger() << F("  • Wi-Fi: ") << endl;
  Interface::get().getLogger() << F("    ◦ SSID: ") << _configStruct.wifi.ssid << endl;
  Interface::get().getLogger() << F("    ◦ Password not shown") << endl;

  Interface::get().getLogger() << F("  • MQTT: ") << endl;
  Interface::get().getLogger() << F("    ◦ Host: ") << _configStruct.mqtt.server.host << endl;
  Interface::get().getLogger() << F("    ◦ Port: ") << _configStruct.mqtt.server.port << endl;
  Interface::get().getLogger() << F("    ◦ Base topic: ") << _configStruct.mqtt.baseTopic << endl;
  Interface::get().getLogger() << F("    ◦ Auth? ") << (_configStruct.mqtt.auth ? F("yes") : F("no")) << endl;
  if (_configStruct.mqtt.auth) {
    Interface::get().getLogger() << F("    ◦ Username: ") << _configStruct.mqtt.username << endl;
    Interface::get().getLogger() << F("    ◦ Password not shown") << endl;
  }

  Interface::get().getLogger() << F("  • OTA: ") << endl;
  Interface::get().getLogger() << F("    ◦ Enabled? ") << (_configStruct.ota.enabled ? F("yes") : F("no")) << endl;
}

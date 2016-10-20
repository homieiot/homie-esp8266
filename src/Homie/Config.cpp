#include "Config.hpp"

using namespace HomieInternals;

Config::Config()
: _bootMode()
, _configStruct()
, _spiffsBegan(false) {
}

bool Config::_spiffsBegin() {
  if (!_spiffsBegan) {
    _spiffsBegan = SPIFFS.begin();
    if (!_spiffsBegan) Interface::get().logger->println(F("✖ Cannot mount filesystem"));
  }

  return _spiffsBegan;
}

bool Config::load() {
  if (!_spiffsBegin()) { return false; }

  _bootMode = BOOT_CONFIG;

  if (!SPIFFS.exists(CONFIG_FILE_PATH)) {
    Interface::get().logger->print(F("✖ "));
    Interface::get().logger->print(CONFIG_FILE_PATH);
    Interface::get().logger->println(F(" doesn't exist"));
    return false;
  }

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  if (!configFile) {
    Interface::get().logger->println(F("✖ Cannot open config file"));
    return false;
  }

  size_t configSize = configFile.size();

  if (configSize > MAX_JSON_CONFIG_FILE_SIZE) {
    Interface::get().logger->println(F("✖ Config file too big"));
    return false;
  }

  char buf[MAX_JSON_CONFIG_FILE_SIZE];
  configFile.readBytes(buf, configSize);
  configFile.close();
  buf[configSize] = '\0';

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& parsedJson = jsonBuffer.parseObject(buf);
  if (!parsedJson.success()) {
    Interface::get().logger->println(F("✖ Invalid JSON in the config file"));
    return false;
  }

  ConfigValidationResult configValidationResult = Validation::validateConfig(parsedJson);
  if (!configValidationResult.valid) {
    Interface::get().logger->print(F("✖ Config file is not valid, reason: "));
    Interface::get().logger->println(configValidationResult.reason);
    return false;
  }

  _bootMode = BOOT_NORMAL;

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
  strcpy(_configStruct.wifi.password, reqWifiPassword);
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
    } else if (iSetting->isUnsignedLong()) {
      HomieSetting<unsigned long>* setting = static_cast<HomieSetting<unsigned long>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        setting->set(settingsObject[setting->getName()].as<unsigned long>());
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
  SPIFFS.remove(CONFIG_BYPASS_STANDALONE_FILE_PATH);
}

void Config::bypassStandalone() {
  if (!_spiffsBegin()) { return; }

  File bypassStandaloneFile = SPIFFS.open(CONFIG_BYPASS_STANDALONE_FILE_PATH, "w");
  if (!bypassStandaloneFile) {
    Interface::get().logger->println(F("✖ Cannot open bypass standalone file"));
    return;
  }

  bypassStandaloneFile.print("1");
  bypassStandaloneFile.close();
}

bool Config::canBypassStandalone() {
  if (!_spiffsBegin()) { return false; }

  return SPIFFS.exists(CONFIG_BYPASS_STANDALONE_FILE_PATH);
}

void Config::write(const JsonObject& config) {
  if (!_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    Interface::get().logger->println(F("✖ Cannot open config file"));
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
      Interface::get().logger->println(F("✖ Invalid or too big JSON"));
      return false;
    }

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    if (!configFile) {
      Interface::get().logger->println(F("✖ Cannot open config file"));
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
            Interface::get().logger->println(error);
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
      Interface::get().logger->print(F("✖ Config file is not valid, reason: "));
      Interface::get().logger->println(configValidationResult.reason);
      return false;
    }

    write(configObject);

    return true;
}

BootMode Config::getBootMode() const {
  return _bootMode;
}

void Config::log() const {
  Interface::get().logger->println(F("{} Stored configuration:"));
  Interface::get().logger->print(F("  • Hardware device ID: "));
  Interface::get().logger->println(DeviceId::get());
  Interface::get().logger->print(F("  • Device ID: "));
  Interface::get().logger->println(_configStruct.deviceId);
  Interface::get().logger->print(F("  • Boot mode: "));
  switch (_bootMode) {
    case BOOT_CONFIG:
      Interface::get().logger->println(F("configuration"));
      break;
    case BOOT_NORMAL:
      Interface::get().logger->println(F("normal"));
      break;
    default:
      Interface::get().logger->println(F("unknown"));
      break;
  }
  Interface::get().logger->print(F("  • Name: "));
  Interface::get().logger->println(_configStruct.name);

  Interface::get().logger->println(F("  • Wi-Fi"));
  Interface::get().logger->print(F("    ◦ SSID: "));
  Interface::get().logger->println(_configStruct.wifi.ssid);
  Interface::get().logger->println(F("    ◦ Password not shown"));

  Interface::get().logger->println(F("  • MQTT"));
  Interface::get().logger->print(F("    ◦ Host: "));
  Interface::get().logger->println(_configStruct.mqtt.server.host);
  Interface::get().logger->print(F("    ◦ Port: "));
  Interface::get().logger->println(_configStruct.mqtt.server.port);
  Interface::get().logger->print(F("    ◦ Base topic: "));
  Interface::get().logger->println(_configStruct.mqtt.baseTopic);
  Interface::get().logger->print(F("    ◦ Auth? "));
  Interface::get().logger->println(_configStruct.mqtt.auth ? F("yes") : F("no"));
  if (_configStruct.mqtt.auth) {
    Interface::get().logger->print(F("    ◦ Username: "));
    Interface::get().logger->println(_configStruct.mqtt.username);
    Interface::get().logger->println(F("    ◦ Password not shown"));
  }

  Interface::get().logger->println(F("  • OTA"));
  Interface::get().logger->print(F("    ◦ Enabled? "));
  Interface::get().logger->println(_configStruct.ota.enabled ? F("yes") : F("no"));
}

#include "Config.hpp"

using namespace HomieInternals;

Config::Config()
: _interface(nullptr)
, _configStruct()
, _spiffsBegan(false) {
}

void Config::attachInterface(Interface* interface) {
  _interface = interface;
}

bool Config::_spiffsBegin() {
  if (!_spiffsBegan) {
    _spiffsBegan = SPIFFS.begin();
    if (!_spiffsBegan) _interface->logger->logln(F("✖ Cannot mount filesystem"));
  }

  return _spiffsBegan;
}

bool Config::load() {
  if (!_spiffsBegin()) { return false; }

  _bootMode = BOOT_CONFIG;

  if (!SPIFFS.exists(CONFIG_FILE_PATH)) {
    _interface->logger->log(F("✖ "));
    _interface->logger->log(CONFIG_FILE_PATH);
    _interface->logger->logln(F(" doesn't exist"));
    return false;
  }

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  if (!configFile) {
    _interface->logger->logln(F("✖ Cannot open config file"));
    return false;
  }

  size_t configSize = configFile.size();

  if (configSize > MAX_JSON_CONFIG_FILE_SIZE) {
    _interface->logger->logln(F("✖ Config file too big"));
    return false;
  }

  char buf[MAX_JSON_CONFIG_FILE_SIZE];
  configFile.readBytes(buf, configSize);
  configFile.close();

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& parsedJson = jsonBuffer.parseObject(buf);
  if (!parsedJson.success()) {
    _interface->logger->logln(F("✖ Invalid JSON in the config file"));
    return false;
  }

  ConfigValidationResult configValidationResult = Helpers::validateConfig(parsedJson);
  if (!configValidationResult.valid) {
    _interface->logger->log(F("✖ Config file is not valid, reason: "));
    _interface->logger->logln(configValidationResult.reason);
    return false;
  }

  _bootMode = BOOT_NORMAL;

  const char* reqName = parsedJson["name"];
  const char* reqWifiSsid = parsedJson["wifi"]["ssid"];
  const char* reqWifiPassword = parsedJson["wifi"]["password"];

  const char* reqMqttHost = parsedJson["mqtt"]["host"];
  const char* reqDeviceId = Helpers::getDeviceId();
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
}

void Config::write(const char* config) {
  if (!_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    _interface->logger->logln(F("✖ Cannot open config file"));
    return;
  }

  configFile.print(config);
  configFile.close();
}

BootMode Config::getBootMode() const {
  return _bootMode;
}

void Config::log() const {
  _interface->logger->logln(F("{} Stored configuration:"));
  _interface->logger->log(F("  • Hardware device ID: "));
  _interface->logger->logln(Helpers::getDeviceId());
  _interface->logger->log(F("  • Device ID: "));
  _interface->logger->logln(_configStruct.deviceId);
  _interface->logger->log(F("  • Boot mode: "));
  switch (_bootMode) {
    case BOOT_CONFIG:
      _interface->logger->logln(F("configuration"));
      break;
    case BOOT_NORMAL:
      _interface->logger->logln(F("normal"));
      break;
    default:
      _interface->logger->logln(F("unknown"));
      break;
  }
  _interface->logger->log(F("  • Name: "));
  _interface->logger->logln(_configStruct.name);

  _interface->logger->logln(F("  • Wi-Fi"));
  _interface->logger->log(F("    ◦ SSID: "));
  _interface->logger->logln(_configStruct.wifi.ssid);
  _interface->logger->logln(F("    ◦ Password not shown"));

  _interface->logger->logln(F("  • MQTT"));
  _interface->logger->log(F("    ◦ Host: "));
  _interface->logger->logln(_configStruct.mqtt.server.host);
  _interface->logger->log(F("    ◦ Port: "));
  _interface->logger->logln(_configStruct.mqtt.server.port);
  _interface->logger->log(F("    ◦ Base topic: "));
  _interface->logger->logln(_configStruct.mqtt.baseTopic);
  _interface->logger->log(F("    ◦ Auth? "));
  _interface->logger->logln(_configStruct.mqtt.auth ? F("yes") : F("no"));
  if (_configStruct.mqtt.auth) {
    _interface->logger->log(F("    ◦ Username: "));
    _interface->logger->logln(_configStruct.mqtt.username);
    _interface->logger->logln(F("    ◦ Password not shown"));
  }

  _interface->logger->logln(F("  • OTA"));
  _interface->logger->log(F("    ◦ Enabled? "));
  _interface->logger->logln(_configStruct.ota.enabled ? F("yes") : F("no"));
}

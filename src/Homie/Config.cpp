#include "Config.hpp"

using namespace HomieInternals;

Config::Config()
  : _configStruct()
  , _spiffsBegan(false)
  , _valid(false) {
}

bool Config::_spiffsBegin() {
  if (!_spiffsBegan) {
#ifdef ESP32
    _spiffsBegan = SPIFFS.begin(true);
#elif defined(ESP8266)
    _spiffsBegan = SPIFFS.begin();
#endif
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

  if (configSize >= MAX_JSON_CONFIG_FILE_SIZE) {
    Interface::get().getLogger() << F("✖ Config file too big") << endl;
    return false;
  }

  char buf[MAX_JSON_CONFIG_FILE_SIZE];
  configFile.readBytes(buf, configSize);
  configFile.close();
  buf[configSize] = '\0';

  StaticJsonDocument<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonDoc;
  if (deserializeJson(jsonDoc, buf) != DeserializationError::Ok || !jsonDoc.is<JsonObject>()) {
    Interface::get().getLogger() << F("✖ Invalid JSON in the config file") << endl;
    return false;
  }

  JsonObject parsedJson = jsonDoc.as<JsonObject>();
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
  uint16_t regDeviceStatsInterval = STATS_SEND_INTERVAL_SEC; //device_stats_interval
  if (parsedJson.containsKey(F("device_stats_interval"))) {
    regDeviceStatsInterval = parsedJson[F("device_stats_interval")];
  }

  const char* reqWifiBssid = "";
  if (parsedJson["wifi"].as<JsonObject>().containsKey("bssid")) {
    reqWifiBssid = parsedJson["wifi"]["bssid"];
  }
  uint16_t reqWifiChannel = 0;
  if (parsedJson["wifi"].as<JsonObject>().containsKey("channel")) {
    reqWifiChannel = parsedJson["wifi"]["channel"];
  }
  const char* reqWifiIp = "";
  if (parsedJson["wifi"].as<JsonObject>().containsKey("ip")) {
    reqWifiIp = parsedJson["wifi"]["ip"];
  }
  const char* reqWifiMask = "";
  if (parsedJson["wifi"].as<JsonObject>().containsKey("mask")) {
    reqWifiMask = parsedJson["wifi"]["mask"];
  }
  const char* reqWifiGw = "";
  if (parsedJson["wifi"].as<JsonObject>().containsKey("gw")) {
    reqWifiGw = parsedJson["wifi"]["gw"];
  }
  const char* reqWifiDns1 = "";
  if (parsedJson["wifi"].as<JsonObject>().containsKey("dns1")) {
    reqWifiDns1 = parsedJson["wifi"]["dns1"];
  }
  const char* reqWifiDns2 = "";
  if (parsedJson["wifi"].as<JsonObject>().containsKey("dns2")) {
    reqWifiDns2 = parsedJson["wifi"]["dns2"];
  }

  uint16_t reqMqttPort = DEFAULT_MQTT_PORT;
  if (parsedJson["mqtt"].as<JsonObject>().containsKey("port")) {
    reqMqttPort = parsedJson["mqtt"]["port"];
  }
  bool reqMqttSsl = false;
  if (parsedJson["mqtt"].as<JsonObject>().containsKey("ssl")) {
    reqMqttSsl = parsedJson["mqtt"]["ssl"];
  }
  const char* reqMqttFingerprint = "";
  if (parsedJson["mqtt"].as<JsonObject>().containsKey("ssl_fingerprint")) {
    reqMqttFingerprint = parsedJson["mqtt"]["ssl_fingerprint"];
  }
  const char* reqMqttBaseTopic = DEFAULT_MQTT_BASE_TOPIC;
  if (parsedJson["mqtt"].as<JsonObject>().containsKey("base_topic")) {
    reqMqttBaseTopic = parsedJson["mqtt"]["base_topic"];
  }
  bool reqMqttAuth = false;
  if (parsedJson["mqtt"].as<JsonObject>().containsKey("auth")) {
    reqMqttAuth = parsedJson["mqtt"]["auth"];
  }
  const char* reqMqttUsername = "";
  if (parsedJson["mqtt"].as<JsonObject>().containsKey("username")) {
    reqMqttUsername = parsedJson["mqtt"]["username"];
  }
  const char* reqMqttPassword = "";
  if (parsedJson["mqtt"].as<JsonObject>().containsKey("password")) {
    reqMqttPassword = parsedJson["mqtt"]["password"];
  }

  bool reqOtaEnabled = false;
  if (parsedJson["ota"].as<JsonObject>().containsKey("enabled")) {
    reqOtaEnabled = parsedJson["ota"]["enabled"];
  }

  strlcpy(_configStruct.name, reqName, MAX_FRIENDLY_NAME_LENGTH);
  strlcpy(_configStruct.deviceId, reqDeviceId, MAX_DEVICE_ID_LENGTH);
  _configStruct.deviceStatsInterval = regDeviceStatsInterval;
  strlcpy(_configStruct.wifi.ssid, reqWifiSsid, MAX_WIFI_SSID_LENGTH);
  if (reqWifiPassword) strlcpy(_configStruct.wifi.password, reqWifiPassword, MAX_WIFI_PASSWORD_LENGTH);
  strlcpy(_configStruct.wifi.bssid, reqWifiBssid, MAX_MAC_STRING_LENGTH + 6);
  _configStruct.wifi.channel = reqWifiChannel;
  strlcpy(_configStruct.wifi.ip, reqWifiIp, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.gw, reqWifiGw, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.mask, reqWifiMask, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.dns1, reqWifiDns1, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.dns2, reqWifiDns2, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.mqtt.server.host, reqMqttHost, MAX_HOSTNAME_LENGTH);
#if ASYNC_TCP_SSL_ENABLED
  _configStruct.mqtt.server.ssl.enabled = reqMqttSsl;
  if (strcmp_P(reqMqttFingerprint, PSTR("")) != 0) {
    _configStruct.mqtt.server.ssl.hasFingerprint = true;
    Helpers::hexStringToByteArray(reqMqttFingerprint, _configStruct.mqtt.server.ssl.fingerprint, MAX_FINGERPRINT_SIZE);
  }
#endif
  _configStruct.mqtt.server.port = reqMqttPort;
  strlcpy(_configStruct.mqtt.baseTopic, reqMqttBaseTopic, MAX_MQTT_BASE_TOPIC_LENGTH);
  _configStruct.mqtt.auth = reqMqttAuth;
  strlcpy(_configStruct.mqtt.username, reqMqttUsername, MAX_MQTT_CREDS_LENGTH);
  strlcpy(_configStruct.mqtt.password, reqMqttPassword, MAX_MQTT_CREDS_LENGTH);
  _configStruct.ota.enabled = reqOtaEnabled;

  /* Parse the settings */

  JsonObject settingsObject = parsedJson["settings"].as<JsonObject>();

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

  StaticJsonDocument<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonDoc;
  deserializeJson(jsonDoc, buf);
  JsonObject parsedJson = jsonDoc.as<JsonObject>();
  parsedJson["wifi"].as<JsonObject>().remove("password");
  parsedJson["mqtt"].as<JsonObject>().remove("username");
  parsedJson["mqtt"].as<JsonObject>().remove("password");

  size_t jsonBufferLength = measureJson(jsonDoc) + 1;
  std::unique_ptr<char[]> jsonString(new char[jsonBufferLength]);
  serializeJson(jsonDoc, jsonString.get(), jsonBufferLength);
  return strdup(jsonString.get());
}

void Config::erase() {
  if (!_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);
  SPIFFS.remove(CONFIG_NEXT_BOOT_MODE_FILE_PATH);
}

void Config::setHomieBootModeOnNextBoot(HomieBootMode bootMode) {
  if (!_spiffsBegin()) { return; }

  if (bootMode == HomieBootMode::UNDEFINED) {
    SPIFFS.remove(CONFIG_NEXT_BOOT_MODE_FILE_PATH);
  } else {
    File bootModeFile = SPIFFS.open(CONFIG_NEXT_BOOT_MODE_FILE_PATH, "w");
    if (!bootModeFile) {
      Interface::get().getLogger() << F("✖ Cannot open NEXTMODE file") << endl;
      return;
    }

    bootModeFile.printf("#%d", bootMode);
    bootModeFile.close();
    Interface::get().getLogger().printf("Setting next boot mode to %d\n", bootMode);
  }
}

HomieBootMode Config::getHomieBootModeOnNextBoot() {
  if (!_spiffsBegin()) { return HomieBootMode::UNDEFINED; }

  File bootModeFile = SPIFFS.open(CONFIG_NEXT_BOOT_MODE_FILE_PATH, "r");
  if (bootModeFile) {
    int v = bootModeFile.parseInt();
    bootModeFile.close();
    return static_cast<HomieBootMode>(v);
  } else {
    return HomieBootMode::UNDEFINED;
  }
}

void Config::write(const JsonObject config) {
  if (!_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    Interface::get().getLogger() << F("✖ Cannot open config file") << endl;
    return;
  }
  serializeJson(config, configFile);
  configFile.close();
}

bool Config::patch(const char* patch) {
  if (!_spiffsBegin()) { return false; }

  StaticJsonDocument<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> patchJsonDoc;

  if (deserializeJson(patchJsonDoc, patch) != DeserializationError::Ok || !patchJsonDoc.is<JsonObject>()) {
    Interface::get().getLogger() << F("✖ Invalid or too big JSON") << endl;
    return false;
  }

  JsonObject patchObject = patchJsonDoc.as<JsonObject>();
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

  StaticJsonDocument<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> configJsonDoc;
  deserializeJson(configJsonDoc, configJson);
  JsonObject configObject = configJsonDoc.as<JsonObject>();

  _patchJsonObject(configObject, patchObject);

  ConfigValidationResult configValidationResult = Validation::validateConfig(configObject);
  if (!configValidationResult.valid) {
    Interface::get().getLogger() << F("✖ Config file is not valid, reason: ") << configValidationResult.reason << endl;
    return false;
  }

  write(configObject);

  return true;
}

void Config::_patchJsonObject(JsonObject object, JsonObject patch) {
  for (JsonPair patchRootElement : patch) {
    JsonVariant patchElement = patchRootElement.value();
    JsonVariant origElement = object[patchRootElement.key()];

    /* Overwrite a config element if it is not existent or not an object or if the patch equivalent is not an object */
    if (origElement.isNull() || !origElement.is<JsonObject>() || !patchElement.is<JsonObject>()) {
      object[patchRootElement.key()] = patchElement;
    } else {
        _patchJsonObject(origElement.as<JsonObject>(), patchElement.as<JsonObject>());
    }
  }
}

bool Config::isValid() const {
  return this->_valid;
}

void Config::log() const {
  Interface::get().getLogger() << F("{} Stored configuration") << endl;
  Interface::get().getLogger() << F("  • Hardware device ID: ") << DeviceId::get() << endl;
  Interface::get().getLogger() << F("  • Device ID: ") << _configStruct.deviceId << endl;
  Interface::get().getLogger() << F("  • Name: ") << _configStruct.name << endl;
  Interface::get().getLogger() << F("  • Device Stats Interval: ") << _configStruct.deviceStatsInterval << F(" sec") << endl;

  Interface::get().getLogger() << F("  • Wi-Fi: ") << endl;
  Interface::get().getLogger() << F("    ◦ SSID: ") << _configStruct.wifi.ssid << endl;
  Interface::get().getLogger() << F("    ◦ Password not shown") << endl;
  if (strcmp_P(_configStruct.wifi.ip, PSTR("")) != 0) {
    Interface::get().getLogger() << F("    ◦ IP: ") << _configStruct.wifi.ip << endl;
    Interface::get().getLogger() << F("    ◦ Mask: ") << _configStruct.wifi.mask << endl;
    Interface::get().getLogger() << F("    ◦ Gateway: ") << _configStruct.wifi.gw << endl;
  }
  Interface::get().getLogger() << F("  • MQTT: ") << endl;
  Interface::get().getLogger() << F("    ◦ Host: ") << _configStruct.mqtt.server.host << endl;
  Interface::get().getLogger() << F("    ◦ Port: ") << _configStruct.mqtt.server.port << endl;
#if ASYNC_TCP_SSL_ENABLED
  Interface::get().getLogger() << F("    ◦ SSL enabled: ") << (_configStruct.mqtt.server.ssl.enabled ? "true" : "false") << endl;
  if (_configStruct.mqtt.server.ssl.enabled && _configStruct.mqtt.server.ssl.hasFingerprint) {
    char hexBuf[MAX_FINGERPRINT_STRING_LENGTH];
    Helpers::byteArrayToHexString(Interface::get().getConfig().get().mqtt.server.ssl.fingerprint, hexBuf, MAX_FINGERPRINT_SIZE);
    Interface::get().getLogger() << F("    ◦ Fingerprint: ") << hexBuf << endl;
  }
#endif
  Interface::get().getLogger() << F("    ◦ Base topic: ") << _configStruct.mqtt.baseTopic << endl;
  Interface::get().getLogger() << F("    ◦ Auth? ") << (_configStruct.mqtt.auth ? F("yes") : F("no")) << endl;
  if (_configStruct.mqtt.auth) {
    Interface::get().getLogger() << F("    ◦ Username: ") << _configStruct.mqtt.username << endl;
    Interface::get().getLogger() << F("    ◦ Password not shown") << endl;
  }

  Interface::get().getLogger() << F("  • OTA: ") << endl;
  Interface::get().getLogger() << F("    ◦ Enabled? ") << (_configStruct.ota.enabled ? F("yes") : F("no")) << endl;

  if (IHomieSetting::settings.size() > 0) {
    Interface::get().getLogger() << F("  • Custom settings: ") << endl;
    for (IHomieSetting* iSetting : IHomieSetting::settings) {
      Interface::get().getLogger() << F("    ◦ ");

      if (iSetting->isBool()) {
        HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);
        Interface::get().getLogger() << setting->getName() << F(": ") << setting->get() << F(" (") << (setting->wasProvided() ? F("set") : F("default")) << F(")");
      } else if (iSetting->isLong()) {
        HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);
        Interface::get().getLogger() << setting->getName() << F(": ") << setting->get() << F(" (") << (setting->wasProvided() ? F("set") : F("default")) << F(")");
      } else if (iSetting->isDouble()) {
        HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);
        Interface::get().getLogger() << setting->getName() << F(": ") << setting->get() << F(" (") << (setting->wasProvided() ? F("set") : F("default")) << F(")");
      } else if (iSetting->isConstChar()) {
        HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);
        Interface::get().getLogger() << setting->getName() << F(": ") << setting->get() << F(" (") << (setting->wasProvided() ? F("set") : F("default")) << F(")");
      }

      Interface::get().getLogger() << endl;
    }
  }
}

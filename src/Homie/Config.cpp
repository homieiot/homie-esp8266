#include "Config.hpp"

using namespace HomieInternals;

Config::Config()
  : _configStruct()
  , _spiffsBegan(false)
  , _valid(false) {
}

const ConfigStruct& Config::get() const {
  return _configStruct;
}

ValidationResultOBJ Config::getJsonObject(StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE>* jsonBuffer) {
  return _loadConfigFile(jsonBuffer, true);
}

bool Config::_spiffsBegin() {
  if (!_spiffsBegan) {
    _spiffsBegan = SPIFFS.begin();
    if (!_spiffsBegan) Interface::get().getLogger() << FPSTR(PROGMEM_CONFIG_SPIFFS_NOT_FOUND) << endl;
  }

  return _spiffsBegan;
}

bool Config::load() {
  _valid = false;

  Interface::get().getLogger() << F("↻ Config file loading") << endl;

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE> jsonBuffer;
  ValidationResultOBJ loadResult = _loadConfigFile(&jsonBuffer);
  if (!loadResult.valid) {
    Interface::get().getLogger() << F("✖ Config file faild to load") << endl;
    Interface::get().getLogger() << loadResult.reason << endl;
    return false;
  }
  JsonObject& parsedJson = *loadResult.config;

  Interface::get().getLogger() << F("✔ Config file loaded and validated") << endl;

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
  if (parsedJson["wifi"].as<JsonObject&>().containsKey("bssid")) {
    reqWifiBssid = parsedJson["wifi"]["bssid"];
  }
  uint16_t reqWifiChannel = 0;
  if (parsedJson["wifi"].as<JsonObject&>().containsKey("channel")) {
    reqWifiChannel = parsedJson["wifi"]["channel"];
  }
  const char* reqWifiIp = "";
  if (parsedJson["wifi"].as<JsonObject&>().containsKey("ip")) {
    reqWifiIp = parsedJson["wifi"]["ip"];
  }
  const char* reqWifiMask = "";
  if (parsedJson["wifi"].as<JsonObject&>().containsKey("mask")) {
    reqWifiMask = parsedJson["wifi"]["mask"];
  }
  const char* reqWifiGw = "";
  if (parsedJson["wifi"].as<JsonObject&>().containsKey("gw")) {
    reqWifiGw = parsedJson["wifi"]["gw"];
  }
  const char* reqWifiDns1 = "";
  if (parsedJson["wifi"].as<JsonObject&>().containsKey("dns1")) {
    reqWifiDns1 = parsedJson["wifi"]["dns1"];
  }
  const char* reqWifiDns2 = "";
  if (parsedJson["wifi"].as<JsonObject&>().containsKey("dns2")) {
    reqWifiDns2 = parsedJson["wifi"]["dns2"];
  }

  uint16_t reqMqttPort = DEFAULT_MQTT_PORT;
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("port")) {
    reqMqttPort = parsedJson["mqtt"]["port"];
  }
  bool reqMqttSsl = false;
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("ssl")) {
    reqMqttSsl = parsedJson["mqtt"]["ssl"];
  }
  const char* reqMqttFingerprint = "";
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("ssl_fingerprint")) {
    reqMqttFingerprint = parsedJson["mqtt"]["ssl_fingerprint"];
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

  strlcpy(_configStruct.name, reqName, MAX_FRIENDLY_NAME_STRING_LENGTH);
  strlcpy(_configStruct.deviceId, reqDeviceId, MAX_DEVICE_ID_STRING_LENGTH);
  _configStruct.deviceStatsInterval = regDeviceStatsInterval;
  strlcpy(_configStruct.wifi.ssid, reqWifiSsid, MAX_WIFI_SSID_STRING_LENGTH);
  if (reqWifiPassword) strlcpy(_configStruct.wifi.password, reqWifiPassword, MAX_WIFI_PASSWORD_STRING_LENGTH);
  strlcpy(_configStruct.wifi.bssid, reqWifiBssid, MAX_MAC_STRING_LENGTH);
  _configStruct.wifi.channel = reqWifiChannel;
  strlcpy(_configStruct.wifi.ip, reqWifiIp, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.gw, reqWifiGw, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.mask, reqWifiMask, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.dns1, reqWifiDns1, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.wifi.dns2, reqWifiDns2, MAX_IP_STRING_LENGTH);
  strlcpy(_configStruct.mqtt.server.host, reqMqttHost, MAX_HOSTNAME_STRING_LENGTH);
#if ASYNC_TCP_SSL_ENABLED
  _configStruct.mqtt.server.ssl.enabled = reqMqttSsl;
  if (strcmp_P(reqMqttFingerprint, PSTR("")) != 0) {
    _configStruct.mqtt.server.ssl.hasFingerprint = true;
    Helpers::hexStringToByteArray(reqMqttFingerprint, _configStruct.mqtt.server.ssl.fingerprint, MAX_FINGERPRINT_SIZE);
  }
#endif
  _configStruct.mqtt.server.port = reqMqttPort;
  strlcpy(_configStruct.mqtt.baseTopic, reqMqttBaseTopic, MAX_MQTT_BASE_TOPIC_STRING_LENGTH);
  _configStruct.mqtt.auth = reqMqttAuth;
  strlcpy(_configStruct.mqtt.username, reqMqttUsername, MAX_MQTT_CREDS_STRING_LENGTH);
  strlcpy(_configStruct.mqtt.password, reqMqttPassword, MAX_MQTT_CREDS_STRING_LENGTH);
  _configStruct.ota.enabled = reqOtaEnabled;

  /* Parse the settings */

  JsonObject& settingsObject = parsedJson["settings"].as<JsonObject&>();

  for (IHomieSetting& iSetting : IHomieSetting::settings) {
    if (iSetting.isBool()) {
      HomieSetting<bool>& setting = static_cast<HomieSetting<bool>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        setting._set(settingsObject[setting.getName()].as<bool>());
      }
    } else if (iSetting.isLong()) {
      HomieSetting<long>& setting = static_cast<HomieSetting<long>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        setting._set(settingsObject[setting.getName()].as<long>());
      }
    } else if (iSetting.isDouble()) {
      HomieSetting<double>& setting = static_cast<HomieSetting<double>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        setting._set(settingsObject[setting.getName()].as<double>());
      }
    } else if (iSetting.isConstChar()) {
      HomieSetting<const char*>& setting = static_cast<HomieSetting<const char*>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        setting._set(strdup(settingsObject[setting.getName()].as<const char*>()));
      }
    }
  }

  _valid = true;
  return true;
}

char* Config::getSafeConfigFile() {
  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE> jsonBuffer;
  ValidationResultOBJ configLoadResult = _loadConfigFile(&jsonBuffer);
  if (!configLoadResult.valid) {
    return nullptr;
  }
  JsonObject& configObject = *configLoadResult.config;
  ValidationResult configValidResult = validateConfig(configObject);
  if (!configValidResult.valid) {
    return nullptr;
  }

  configObject["wifi"].as<JsonObject&>().remove("password");
  configObject["mqtt"].as<JsonObject&>().remove("username");
  configObject["mqtt"].as<JsonObject&>().remove("password");

  size_t jsonBufferLength = configObject.measureLength() + 1;
  std::unique_ptr<char[]> jsonString(new char[jsonBufferLength]);
  configObject.printTo(jsonString.get(), jsonBufferLength);

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

ValidationResultOBJ Config::_loadConfigFile(StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE>* jsonBuffer, bool skipValidation) {
  ValidationResultOBJ result;
  result.valid = false;

  if (!_spiffsBegin()) {
    result.reason = FPSTR(PROGMEM_CONFIG_SPIFFS_NOT_FOUND);
    return result;
  }

  if (!SPIFFS.exists(CONFIG_FILE_PATH)) {
    result.reason = F("✖ ");
    result.reason.concat(CONFIG_FILE_PATH);
    result.reason.concat(F(" doesn't exist"));
    return result;
  }

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  if (!configFile) {
    result.reason = FPSTR(PROGMEM_CONFIG_FILE_NOT_FOUND);
    return result;
  }

  size_t configSize = configFile.size();
  if (configSize > MAX_JSON_CONFIG_FILE_SIZE) {
    configFile.close();
    result.reason = FPSTR(PROGMEM_CONFIG_FILE_TOO_BIG);
    return result;
  }

  JsonObject& config = jsonBuffer->parseObject(configFile);

  ValidationResult configValidationResult = validateConfig(config, skipValidation);

  if (!configValidationResult.valid) {
    result.reason = configValidationResult.reason;
    return result;
  }

  result.valid = true;
  result.config = &config;

  return result;
}

ValidationResult Config::validateConfig(const JsonObject& parsedJson, bool skipValidation) {
  ValidationResult result;
  result.valid = false;

  if (!parsedJson.success()) {
    result.reason = F("✖ Invalid or too big JSON");
    return result;
  }

  if (!skipValidation) {
    ValidationResult configValidationResult = Validation::validateConfig(parsedJson);
    if (!configValidationResult.valid) {
      result.reason = F("✖ Config file is not valid, reason: ");
      result.reason.concat(configValidationResult.reason);
      return result;
    }
  }

  result.valid = true;
  return result;
}

ValidationResult Config::write(const JsonObject& newConfig) {
  ValidationResult result;
  result.valid = false;

  ValidationResult validResult = validateConfig(newConfig);
  if (!validResult.valid) {
    result.reason = validResult.reason;
    return result;
  }

  size_t configSize = newConfig.measureLength();
  if (configSize > MAX_JSON_CONFIG_FILE_SIZE) {
    result.reason = FPSTR(PROGMEM_CONFIG_FILE_TOO_BIG);
    result.reason.concat(F(" Size: "));
    result.reason.concat(configSize);
    return result;
  }

  if (!_spiffsBegin()) {
    result.reason = FPSTR(PROGMEM_CONFIG_SPIFFS_NOT_FOUND);
    return result;
  }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    result.reason = FPSTR(PROGMEM_CONFIG_FILE_NOT_FOUND);
    return result;
  }

  // Write new config
  newConfig.printTo(configFile);
  configFile.close();

  Interface::get().getLogger() << F("💾 Saved config file.") << endl;

  result.valid = true;
  return result;
}

ValidationResult Config::patch(const char* patch) {
  ValidationResult result;
  result.valid = false;

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> patchJsonBuffer;
  JsonObject& patchObject = patchJsonBuffer.parseObject(patch);

  // Validate Patch config
  ValidationResult patchValidResult = validateConfig(patchObject, true);
  if (!patchValidResult.valid) {
    result.reason = F("✖ Patch Config file is not valid, reason: ");
    result.reason.concat(patchValidResult.reason);
    return result;
  }

  // Validate current config
  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_FILE_BUFFER_SIZE> currentJsonBuffer;
  ValidationResultOBJ configLoadResult = _loadConfigFile(&currentJsonBuffer);
  if (!configLoadResult.valid) {
    result.reason = F("✖ Old Config file is not valid, reason: ");
    result.reason.concat(configLoadResult.reason);
    return result;
  }
  JsonObject& configObject = *configLoadResult.config;

  for (const JsonPair& patchPair : patchObject) {
    if (patchPair.value.is<JsonObject>() && patchPair.value.size() > 0) {
      if (!configObject.containsKey(patchPair.key)) {
        configObject.createNestedObject(patchPair.key);
      }
      if (!configObject[patchPair.key].is<JsonObject>()) {
        configObject.remove(patchPair.key);
        configObject.createNestedObject(patchPair.key);
      }
      JsonObject& patchSubObject = patchPair.value.as<JsonObject>();
      JsonObject& configSubObject = configObject[patchPair.key].as<JsonObject>();

      for (const JsonPair& subPatchPair : patchSubObject) {
        configSubObject[subPatchPair.key] = subPatchPair.value;
      }
    } else if (patchPair.value.is<JsonArray>()) {
      // Not you handled
      configObject[patchPair.key] = patchPair.value;
    } else {
      configObject[patchPair.key] = patchPair.value;
    }
  }

  ValidationResult configWriteResult = write(configObject);
  if (!configWriteResult.valid) {
    result.reason = F("✖ New Config file is not valid, reason: ");
    result.reason.concat(configWriteResult.reason);
    return result;
  }

  result.valid = true;
  return result;
}

bool Config::isValid() const {
  return this->_valid;
}

void Config::log() const {
  Interface::get().getLogger() << F("{} Stored configuration") << endl;
  Interface::get().getLogger() << F("  • Hardware Device ID: ") << DeviceId::get() << endl;
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
  Interface::get().getLogger() << F("    ◦ SSL Enabled?: ") << (_configStruct.mqtt.server.ssl.enabled ? F("yes") : F("no")) << endl;
  if (_configStruct.mqtt.server.ssl.enabled && _configStruct.mqtt.server.ssl.hasFingerprint) {
    char hexBuf[MAX_FINGERPRINT_STRING_LENGTH];
    Helpers::byteArrayToHexString(Interface::get().getConfig().get().mqtt.server.ssl.fingerprint, hexBuf, MAX_FINGERPRINT_SIZE);
    Interface::get().getLogger() << F("    ◦ Fingerprint: ") << hexBuf << endl;
  }
#endif
  Interface::get().getLogger() << F("    ◦ Base Topic: ") << _configStruct.mqtt.baseTopic << endl;
  Interface::get().getLogger() << F("    ◦ Auth? ") << (_configStruct.mqtt.auth ? F("yes") : F("no")) << endl;
  if (_configStruct.mqtt.auth) {
    Interface::get().getLogger() << F("    ◦ Username: ") << _configStruct.mqtt.username << endl;
    Interface::get().getLogger() << F("    ◦ Password not shown") << endl;
  }

  Interface::get().getLogger() << F("  • OTA: ") << endl;
  Interface::get().getLogger() << F("    ◦ Enabled? ") << (_configStruct.ota.enabled ? F("yes") : F("no")) << endl;

  if (IHomieSetting::settings.size() > 0) {
    Interface::get().getLogger() << F("  • Custom Settings: ") << endl;
    for (IHomieSetting& iSetting : IHomieSetting::settings) {
      Interface::get().getLogger() << F("    ◦ ");

      if (iSetting.isBool()) {
        HomieSetting<bool>& setting = static_cast<HomieSetting<bool>&>(iSetting);
        Interface::get().getLogger() << setting.getName() << F(": ") << setting.get() << F(" (") << (setting.wasProvided() ? F("set") : F("default")) << F(")");
      } else if (iSetting.isLong()) {
        HomieSetting<long>& setting = static_cast<HomieSetting<long>&>(iSetting);
        Interface::get().getLogger() << setting.getName() << F(": ") << setting.get() << F(" (") << (setting.wasProvided() ? F("set") : F("default")) << F(")");
      } else if (iSetting.isDouble()) {
        HomieSetting<double>& setting = static_cast<HomieSetting<double>&>(iSetting);
        Interface::get().getLogger() << setting.getName() << F(": ") << setting.get() << F(" (") << (setting.wasProvided() ? F("set") : F("default")) << F(")");
      } else if (iSetting.isConstChar()) {
        HomieSetting<const char*>& setting = static_cast<HomieSetting<const char*>&>(iSetting);
        Interface::get().getLogger() << setting.getName() << F(": ") << setting.get() << F(" (") << (setting.wasProvided() ? F("set") : F("default")) << F(")");
      } else {
        Interface::get().getLogger() << iSetting.getName() << F(": unknown type: ") << iSetting.getType();
      }

      Interface::get().getLogger() << endl;
    }
  }
}

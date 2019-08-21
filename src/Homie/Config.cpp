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

  // Mandatory config objects
  JsonObject objReqWiFi = parsedJson["wifi"].as<JsonObject>();
  JsonObject objReqMqtt = parsedJson["mqtt"].as<JsonObject>();
  JsonObject objReqOta = parsedJson["ota"].as<JsonObject>();

  // Mandatory config items
  const char* reqName = parsedJson["name"];
  const char* reqWifiSsid = objReqWiFi["ssid"];
  const char* reqWifiPassword = objReqWiFi["password"];
  const char* reqMqttHost = objReqMqtt["host"];

  // Defaults for optional config items
  const char* reqDeviceId = DeviceId::get();
  uint16_t reqDeviceStatsInterval = STATS_SEND_INTERVAL_SEC; //device_stats_interval
  const char* reqWifiBssid = "";
  uint16_t reqWifiChannel = 0;
  const char* reqWifiIp = "";
  const char* reqWifiMask = "";
  const char* reqWifiGw = "";
  const char* reqWifiDns1 = "";
  const char* reqWifiDns2 = "";
  uint16_t reqMqttPort = DEFAULT_MQTT_PORT;
  bool reqMqttSsl = false;
  const char* reqMqttFingerprint = "";
  const char* reqMqttBaseTopic = DEFAULT_MQTT_BASE_TOPIC;
  bool reqMqttAuth = false;
  const char* reqMqttUsername = "";
  const char* reqMqttPassword = "";
  bool reqOtaEnabled = false;

  // JsonVariants for optional config items
  JsonVariant jvReqDeviceId = parsedJson["device_id"];
  JsonVariant jvReqDeviceStatsInterval = parsedJson["device_stats_interval"];
  JsonVariant jvReqWifiBssid = objReqWiFi["bssid"];
  JsonVariant jvReqWiFiChannel = objReqWiFi["channel"];
  JsonVariant jvReqWiFiIp = objReqWiFi["ip"];
  JsonVariant jvReqWiFiMask = objReqWiFi["mask"];
  JsonVariant jvReqWiFiGw = objReqWiFi["gw"];
  JsonVariant jvReqWiFiDns1 = objReqWiFi["dns1"];
  JsonVariant jvReqWiFiDns2 = objReqWiFi["dns2"];
  JsonVariant jvReqMqttPort = objReqMqtt["port"];
  JsonVariant jvReqMqttSsl = objReqMqtt["ssl"];
  JsonVariant jvReqMqttFingerprint = objReqMqtt["ssl_fingerprint"];
  JsonVariant jvReqMqttBaseTopic = objReqMqtt["base_topic"];
  JsonVariant jvReqMqttAuth = objReqMqtt["auth"];
  JsonVariant jvReqMqttUsername = objReqMqtt["username"];
  JsonVariant jvReqMqttPassword = objReqMqtt["password"];
  JsonVariant jvReqOtaEnabled = objReqOta["enabled"];

  if (!jvReqDeviceId.isNull())
    reqDeviceId = jvReqDeviceId.as<const char*>();

  if (!jvReqDeviceStatsInterval.isNull())
    reqDeviceStatsInterval = jvReqDeviceStatsInterval.as<uint16_t>();

  if (!jvReqWifiBssid.isNull())
    reqWifiBssid = jvReqWifiBssid.as<const char*>();

  if (!jvReqWiFiChannel.isNull())
    reqWifiChannel = jvReqWiFiChannel.as<uint16_t>();

  if (!jvReqWiFiIp.isNull())
    reqWifiIp = jvReqWiFiIp.as<const char*>();

  if (!jvReqWiFiMask.isNull())
    reqWifiMask = jvReqWiFiMask.as<const char*>();

  if (!jvReqWiFiGw.isNull())
    reqWifiGw = jvReqWiFiGw.as<const char*>();

  if (!jvReqWiFiDns1.isNull())
    reqWifiDns1 = jvReqWiFiDns1.as<const char*>();

  if (!jvReqWiFiDns2.isNull())
    reqWifiDns2 = jvReqWiFiDns2.as<const char*>();

  if (!jvReqMqttPort.isNull())
    reqMqttPort = jvReqMqttPort.as<uint16_t>();

  if (!jvReqMqttSsl.isNull())
    reqMqttSsl = jvReqMqttSsl.as<bool>();

  if (!jvReqMqttFingerprint.isNull())
    reqMqttFingerprint = jvReqMqttFingerprint.as<const char*>();

  if (!jvReqMqttBaseTopic.isNull())
    reqMqttBaseTopic = jvReqMqttBaseTopic.as<const char*>();

  if (!jvReqMqttAuth.isNull())
    reqMqttAuth = jvReqMqttAuth.as<bool>();

  if (!jvReqMqttUsername.isNull())
    reqMqttUsername = jvReqMqttUsername.as<const char*>();

  if (!jvReqMqttPassword.isNull())
    reqMqttPassword = jvReqMqttPassword.as<const char*>();

  if (!jvReqOtaEnabled.isNull())
    reqOtaEnabled = jvReqOtaEnabled.as<bool>();

  strlcpy(_configStruct.name, reqName, MAX_FRIENDLY_NAME_LENGTH);
  strlcpy(_configStruct.deviceId, reqDeviceId, MAX_DEVICE_ID_LENGTH);
  _configStruct.deviceStatsInterval = reqDeviceStatsInterval;
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
    JsonVariant jvSetting = settingsObject[iSetting->getName()];

    if (!jvSetting.isNull()) {
      if (iSetting->isBool()) {
        HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);
        setting->set(jvSetting.as<bool>());
      } else if (iSetting->isLong()) {
        HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);
        setting->set(jvSetting.as<long>());
      } else if (iSetting->isDouble()) {
        HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);
        setting->set(jvSetting.as<double>());
      } else if (iSetting->isConstChar()) {
        HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);
        setting->set(strdup(jvSetting.as<const char*>()));
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

  // ToDo: allow patching on a higher depth using recursive functions
  for (JsonObject::iterator patchRootElement = patchObject.begin(); patchRootElement != patchObject.end(); ++patchRootElement) {
    if (patchObject[patchRootElement->key()].is<JsonObject>()) {
      JsonObject patchRootObject = patchObject[patchRootElement->key()];
      for (JsonObject::iterator patchSubElement = patchRootObject.begin(); patchSubElement != patchRootObject.end(); ++patchSubElement) {
        JsonVariant configRootElement = configObject[patchSubElement->key()];

        if (!configRootElement.isNull() && configRootElement.is<JsonObject>()) {
          configRootElement[patchSubElement->key()] = patchSubElement->value();
        }  // Allow overwriting null objects:
        else if (configRootElement.isNull() || (configRootElement.is<const char*>() && !configRootElement.as<const char*>())) {
          configObject[patchRootElement->key()] = patchRootObject;
        } else {
            String error = "✖ Config already contains a ";
            error.concat(patchRootElement->key().c_str());
            error.concat(" element which is not an object");
            Interface::get().getLogger() << error << endl;
            return false;
        }
      }
    } else {
      configObject[patchRootElement->key()] = patchRootElement->value();
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

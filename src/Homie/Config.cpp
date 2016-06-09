#include "Config.hpp"

using namespace HomieInternals;

Config::Config()
: _interface(nullptr)
, _configStruct()
, _otaVersion {'\0'}
, _spiffsBegan(false)
{
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

  if (configSize > MAX_JSON_CONFIG_FILE_BUFFER_SIZE) {
    _interface->logger->logln(F("✖ Config file too big"));
    return false;
  }

  char buf[MAX_JSON_CONFIG_FILE_BUFFER_SIZE];
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

  if (SPIFFS.exists(CONFIG_OTA_PATH)) {
    _bootMode = BOOT_OTA;

    File otaFile = SPIFFS.open(CONFIG_OTA_PATH, "r");
    if (otaFile) {
      size_t otaSize = otaFile.size();
      otaFile.readBytes(_otaVersion, otaSize);
      otaFile.close();
    } else {
      _interface->logger->logln(F("✖ Cannot open OTA file"));
    }
  } else {
    _bootMode = BOOT_NORMAL;
  }

  const char* reqName = parsedJson["name"];
  const char* reqWifiSsid = parsedJson["wifi"]["ssid"];
  const char* reqWifiPassword = parsedJson["wifi"]["password"];
  bool reqMqttMdns = false;
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("mdns")) reqMqttMdns = true;
  bool reqOtaMdns = false;
  if (parsedJson["ota"].as<JsonObject&>().containsKey("mdns")) reqOtaMdns = true;

  const char* reqMqttHost = "";
  const char* reqMqttMdnsService = "";
  if (reqMqttMdns) {
    reqMqttMdnsService = parsedJson["mqtt"]["mdns"];
  } else {
    reqMqttHost = parsedJson["mqtt"]["host"];
  }
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
  bool reqMqttSsl = false;
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("ssl")) {
    reqMqttSsl = parsedJson["mqtt"]["ssl"];
  }
  const char* reqMqttFingerprint = "";
  if (parsedJson["mqtt"].as<JsonObject&>().containsKey("fingerprint")) {
    reqMqttFingerprint = parsedJson["mqtt"]["fingerprint"];
  }
  bool reqOtaEnabled = false;
  if (parsedJson["ota"].as<JsonObject&>().containsKey("enabled")) {
    reqOtaEnabled = parsedJson["ota"]["enabled"];
  }
  const char* reqOtaHost = reqMqttHost;
  const char* reqOtaMdnsService = "";
  if (reqOtaMdns) {
    reqOtaMdnsService = parsedJson["ota"]["mdns"];
  } else if (parsedJson["ota"].as<JsonObject&>().containsKey("host")) {
    reqOtaHost = parsedJson["ota"]["host"];
  }
  uint16_t reqOtaPort = DEFAULT_OTA_PORT;
  if (parsedJson["ota"].as<JsonObject&>().containsKey("port")) {
    reqOtaPort = parsedJson["ota"]["port"];
  }
  const char* reqOtaPath = DEFAULT_OTA_PATH;
  if (parsedJson["ota"].as<JsonObject&>().containsKey("path")) {
    reqOtaPath = parsedJson["ota"]["path"];
  }
  bool reqOtaSsl = false;
  if (parsedJson["ota"].as<JsonObject&>().containsKey("ssl")) {
    reqOtaSsl = parsedJson["ota"]["ssl"];
  }
  const char* reqOtaFingerprint = "";
  if (parsedJson["ota"].as<JsonObject&>().containsKey("fingerprint")) {
    reqOtaFingerprint = parsedJson["ota"]["fingerprint"];
  }

  strcpy(_configStruct.name, reqName);
  strcpy(_configStruct.wifi.ssid, reqWifiSsid);
  strcpy(_configStruct.wifi.password, reqWifiPassword);
  strcpy(_configStruct.deviceId, reqDeviceId);
  strcpy(_configStruct.mqtt.server.host, reqMqttHost);
  _configStruct.mqtt.server.port = reqMqttPort;
  _configStruct.mqtt.server.mdns.enabled = reqMqttMdns;
  strcpy(_configStruct.mqtt.server.mdns.service, reqMqttMdnsService);
  strcpy(_configStruct.mqtt.baseTopic, reqMqttBaseTopic);
  _configStruct.mqtt.auth = reqMqttAuth;
  strcpy(_configStruct.mqtt.username, reqMqttUsername);
  strcpy(_configStruct.mqtt.password, reqMqttPassword);
  _configStruct.mqtt.server.ssl.enabled = reqMqttSsl;
  strcpy(_configStruct.mqtt.server.ssl.fingerprint, reqMqttFingerprint);
  _configStruct.ota.enabled = reqOtaEnabled;
  strcpy(_configStruct.ota.server.host, reqOtaHost);
  _configStruct.ota.server.port = reqOtaPort;
  _configStruct.ota.server.mdns.enabled = reqOtaMdns;
  strcpy(_configStruct.ota.server.mdns.service, reqOtaMdnsService);
  strcpy(_configStruct.ota.path, reqOtaPath);
  _configStruct.ota.server.ssl.enabled = reqOtaSsl;
  strcpy(_configStruct.ota.server.ssl.fingerprint, reqOtaFingerprint);

  return true;
}

void Config::erase() {
  if (!_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);
  SPIFFS.remove(CONFIG_OTA_PATH);
}

void Config::write(const String& config) {
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

void Config::setOtaMode(bool enabled, const char* version) {
  if (!_spiffsBegin()) { return; }

  if (enabled) {
    File otaFile = SPIFFS.open(CONFIG_OTA_PATH, "w");
    if (!otaFile) {
      _interface->logger->logln(F("✖ Cannot open OTA file"));
      return;
    }

    otaFile.print(version);
    otaFile.close();
  } else {
    SPIFFS.remove(CONFIG_OTA_PATH);
  }
}

const char* Config::getOtaVersion() const {
  return _otaVersion;
}

BootMode Config::getBootMode() const {
  return _bootMode;
}

void Config::log() {
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
    case BOOT_OTA:
      _interface->logger->logln(F("OTA"));
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
  if (_configStruct.mqtt.server.mdns.enabled) {
    _interface->logger->log(F("    ◦ mDNS: "));
    _interface->logger->log(_configStruct.mqtt.server.mdns.service);
  } else {
    _interface->logger->log(F("    ◦ Host: "));
    _interface->logger->logln(_configStruct.mqtt.server.host);
    _interface->logger->log(F("    ◦ Port: "));
    _interface->logger->logln(_configStruct.mqtt.server.port);
  }
  _interface->logger->log(F("    ◦ Base topic: "));
  _interface->logger->logln(_configStruct.mqtt.baseTopic);
  _interface->logger->log(F("    ◦ Auth? "));
  _interface->logger->logln(_configStruct.mqtt.auth ? F("yes") : F("no"));
  if (_configStruct.mqtt.auth) {
    _interface->logger->log(F("    ◦ Username: "));
    _interface->logger->logln(_configStruct.mqtt.username);
    _interface->logger->logln(F("    ◦ Password not shown"));
  }
  _interface->logger->log(F("    ◦ SSL? "));
  _interface->logger->logln(_configStruct.mqtt.server.ssl.enabled ? F("yes") : F("no"));
  if (_configStruct.mqtt.server.ssl.enabled) {
    _interface->logger->log(F("    ◦ Fingerprint: "));
    if (strcmp_P(_configStruct.mqtt.server.ssl.fingerprint, PSTR("")) == 0) _interface->logger->logln(F("unset"));
    else _interface->logger->logln(_configStruct.mqtt.server.ssl.fingerprint);
  }

  _interface->logger->logln(F("  • OTA"));
  _interface->logger->log(F("    ◦ Enabled? "));
  _interface->logger->logln(_configStruct.ota.enabled ? F("yes") : F("no"));
  if (_configStruct.ota.enabled) {
    if (_configStruct.ota.server.mdns.enabled) {
      _interface->logger->log(F("    ◦ mDNS: "));
      _interface->logger->log(_configStruct.ota.server.mdns.service);
    } else {
      _interface->logger->log(F("    ◦ Host: "));
      _interface->logger->logln(_configStruct.ota.server.host);
      _interface->logger->log(F("    ◦ Port: "));
      _interface->logger->logln(_configStruct.ota.server.port);
    }
    _interface->logger->log(F("    ◦ Path: "));
    _interface->logger->logln(_configStruct.ota.path);
    _interface->logger->log(F("    ◦ SSL? "));
    _interface->logger->logln(_configStruct.ota.server.ssl.enabled ? F("yes") : F("no"));
    if (_configStruct.ota.server.ssl.enabled) {
      _interface->logger->log(F("    ◦ Fingerprint: "));
      if (strcmp_P(_configStruct.ota.server.ssl.fingerprint, PSTR("")) == 0) _interface->logger->logln(F("unset"));
      else _interface->logger->logln(_configStruct.ota.server.ssl.fingerprint);
    }
  }
}

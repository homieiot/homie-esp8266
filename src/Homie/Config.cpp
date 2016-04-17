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
  this->_interface = interface;
}

bool Config::_spiffsBegin() {
  if (!this->_spiffsBegan) {
    this->_spiffsBegan = SPIFFS.begin();
    if (!this->_spiffsBegan) this->_interface->logger->logln(F("✖ Cannot mount filesystem"));
  }

  return this->_spiffsBegan;
}

bool Config::load() {
  if (!this->_spiffsBegin()) { return false; }

  this->_bootMode = BOOT_CONFIG;

  if (!SPIFFS.exists(CONFIG_FILE_PATH)) {
    this->_interface->logger->log(F("✖ "));
    this->_interface->logger->log(CONFIG_FILE_PATH);
    this->_interface->logger->logln(F(" doesn't exist"));
    return false;
  }

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  if (!configFile) {
    this->_interface->logger->logln(F("✖ Cannot open config file"));
    return false;
  }

  size_t configSize = configFile.size();

  if (configSize > MAX_JSON_CONFIG_FILE_BUFFER_SIZE) {
    this->_interface->logger->logln(F("✖ Config file too big"));
    return false;
  }

  char buf[MAX_JSON_CONFIG_FILE_BUFFER_SIZE];
  configFile.readBytes(buf, configSize);
  configFile.close();

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& parsedJson = jsonBuffer.parseObject(buf);
  if (!parsedJson.success()) {
    this->_interface->logger->logln(F("✖ Invalid JSON in the config file"));
    return false;
  }

  ConfigValidationResult configValidationResult = Helpers::validateConfig(parsedJson);
  if (!configValidationResult.valid) {
    this->_interface->logger->log(F("✖ Config file is not valid, reason: "));
    this->_interface->logger->logln(configValidationResult.reason);
    return false;
  }

  if (SPIFFS.exists(CONFIG_OTA_PATH)) {
    this->_bootMode = BOOT_OTA;

    File otaFile = SPIFFS.open(CONFIG_OTA_PATH, "r");
    if (otaFile) {
      size_t otaSize = otaFile.size();
      otaFile.readBytes(this->_otaVersion, otaSize);
      otaFile.close();
    } else {
      this->_interface->logger->logln(F("✖ Cannot open OTA file"));
    }
  } else {
    this->_bootMode = BOOT_NORMAL;
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
  unsigned int reqMqttPort = DEFAULT_MQTT_PORT;
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
  unsigned int reqOtaPort = DEFAULT_OTA_PORT;
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

  strcpy(this->_configStruct.name, reqName);
  strcpy(this->_configStruct.wifi.ssid, reqWifiSsid);
  strcpy(this->_configStruct.wifi.password, reqWifiPassword);
  strcpy(this->_configStruct.deviceId, reqDeviceId);
  strcpy(this->_configStruct.mqtt.server.host, reqMqttHost);
  this->_configStruct.mqtt.server.port = reqMqttPort;
  this->_configStruct.mqtt.server.mdns.enabled = reqMqttMdns;
  strcpy(this->_configStruct.mqtt.server.mdns.service, reqMqttMdnsService);
  strcpy(this->_configStruct.mqtt.baseTopic, reqMqttBaseTopic);
  this->_configStruct.mqtt.auth = reqMqttAuth;
  strcpy(this->_configStruct.mqtt.username, reqMqttUsername);
  strcpy(this->_configStruct.mqtt.password, reqMqttPassword);
  this->_configStruct.mqtt.server.ssl.enabled = reqMqttSsl;
  strcpy(this->_configStruct.mqtt.server.ssl.fingerprint, reqMqttFingerprint);
  this->_configStruct.ota.enabled = reqOtaEnabled;
  strcpy(this->_configStruct.ota.server.host, reqOtaHost);
  this->_configStruct.ota.server.port = reqOtaPort;
  this->_configStruct.ota.server.mdns.enabled = reqOtaMdns;
  strcpy(this->_configStruct.ota.server.mdns.service, reqOtaMdnsService);
  strcpy(this->_configStruct.ota.path, reqOtaPath);
  this->_configStruct.ota.server.ssl.enabled = reqOtaSsl;
  strcpy(this->_configStruct.ota.server.ssl.fingerprint, reqOtaFingerprint);

  return true;
}

const ConfigStruct& Config::get() {
  return this->_configStruct;
}

void Config::erase() {
  if (!this->_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);
  SPIFFS.remove(CONFIG_OTA_PATH);
}

void Config::write(const String& config) {
  if (!this->_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    this->_interface->logger->logln(F("✖ Cannot open config file"));
    return;
  }

  configFile.print(config);
  configFile.close();
}

void Config::setOtaMode(bool enabled, const char* version) {
  if (!this->_spiffsBegin()) { return; }

  if (enabled) {
    File otaFile = SPIFFS.open(CONFIG_OTA_PATH, "w");
    if (!otaFile) {
      this->_interface->logger->logln(F("✖ Cannot open OTA file"));
      return;
    }

    otaFile.print(version);
    otaFile.close();
  } else {
    SPIFFS.remove(CONFIG_OTA_PATH);
  }
}

const char* Config::getOtaVersion() {
  return this->_otaVersion;
}

BootMode Config::getBootMode() {
  return this->_bootMode;
}

void Config::log() {
  this->_interface->logger->logln(F("{} Stored configuration:"));
  this->_interface->logger->log(F("  • Hardware device ID: "));
  this->_interface->logger->logln(Helpers::getDeviceId());
  this->_interface->logger->log(F("  • Device ID: "));
  this->_interface->logger->logln(this->_configStruct.deviceId);
  this->_interface->logger->log(F("  • Boot mode: "));
  switch (this->_bootMode) {
    case BOOT_CONFIG:
      this->_interface->logger->logln(F("configuration"));
      break;
    case BOOT_NORMAL:
      this->_interface->logger->logln(F("normal"));
      break;
    case BOOT_OTA:
      this->_interface->logger->logln(F("OTA"));
      break;
    default:
      this->_interface->logger->logln(F("unknown"));
      break;
  }
  this->_interface->logger->log(F("  • Name: "));
  this->_interface->logger->logln(this->_configStruct.name);

  this->_interface->logger->logln(F("  • Wi-Fi"));
  this->_interface->logger->log(F("    ◦ SSID: "));
  this->_interface->logger->logln(this->_configStruct.wifi.ssid);
  this->_interface->logger->logln(F("    ◦ Password not shown"));

  this->_interface->logger->logln(F("  • MQTT"));
  if (this->_configStruct.mqtt.server.mdns.enabled) {
    this->_interface->logger->log(F("    ◦ mDNS: "));
    this->_interface->logger->log(this->_configStruct.mqtt.server.mdns.service);
  } else {
    this->_interface->logger->log(F("    ◦ Host: "));
    this->_interface->logger->logln(this->_configStruct.mqtt.server.host);
    this->_interface->logger->log(F("    ◦ Port: "));
    this->_interface->logger->logln(this->_configStruct.mqtt.server.port);
  }
  this->_interface->logger->log(F("    ◦ Base topic: "));
  this->_interface->logger->logln(this->_configStruct.mqtt.baseTopic);
  this->_interface->logger->log(F("    ◦ Auth? "));
  this->_interface->logger->logln(this->_configStruct.mqtt.auth ? F("yes") : F("no"));
  if (this->_configStruct.mqtt.auth) {
    this->_interface->logger->log(F("    ◦ Username: "));
    this->_interface->logger->logln(this->_configStruct.mqtt.username);
    this->_interface->logger->logln(F("    ◦ Password not shown"));
  }
  this->_interface->logger->log(F("    ◦ SSL? "));
  this->_interface->logger->logln(this->_configStruct.mqtt.server.ssl.enabled ? F("yes") : F("no"));
  if (this->_configStruct.mqtt.server.ssl.enabled) {
    this->_interface->logger->log(F("    ◦ Fingerprint: "));
    if (strcmp_P(this->_configStruct.mqtt.server.ssl.fingerprint, PSTR("")) == 0) this->_interface->logger->logln(F("unset"));
    else this->_interface->logger->logln(this->_configStruct.mqtt.server.ssl.fingerprint);
  }

  this->_interface->logger->logln(F("  • OTA"));
  this->_interface->logger->log(F("    ◦ Enabled? "));
  this->_interface->logger->logln(this->_configStruct.ota.enabled ? F("yes") : F("no"));
  if (this->_configStruct.ota.enabled) {
    if (this->_configStruct.ota.server.mdns.enabled) {
      this->_interface->logger->log(F("    ◦ mDNS: "));
      this->_interface->logger->log(this->_configStruct.ota.server.mdns.service);
    } else {
      this->_interface->logger->log(F("    ◦ Host: "));
      this->_interface->logger->logln(this->_configStruct.ota.server.host);
      this->_interface->logger->log(F("    ◦ Port: "));
      this->_interface->logger->logln(this->_configStruct.ota.server.port);
    }
    this->_interface->logger->log(F("    ◦ Path: "));
    this->_interface->logger->logln(this->_configStruct.ota.path);
    this->_interface->logger->log(F("    ◦ SSL? "));
    this->_interface->logger->logln(this->_configStruct.ota.server.ssl.enabled ? F("yes") : F("no"));
    if (this->_configStruct.ota.server.ssl.enabled) {
      this->_interface->logger->log(F("    ◦ Fingerprint: "));
      if (strcmp_P(this->_configStruct.ota.server.ssl.fingerprint, PSTR("")) == 0) this->_interface->logger->logln(F("unset"));
      else this->_interface->logger->logln(this->_configStruct.ota.server.ssl.fingerprint);
    }
  }
}

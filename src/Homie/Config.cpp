#include "Config.hpp"

using namespace HomieInternals;

ConfigClass::ConfigClass()
: _configStruct()
, _otaVersion {'\0'}
, _spiffsBegan(false)
{
}

bool ConfigClass::_spiffsBegin() {
  if (!this->_spiffsBegan) {
    this->_spiffsBegan = SPIFFS.begin();
    if (!this->_spiffsBegan) Logger.logln(F("✖ Cannot mount filesystem"));
  }

  return this->_spiffsBegan;
}

bool ConfigClass::load() {
  if (!this->_spiffsBegin()) { return false; }

  this->_bootMode = BOOT_CONFIG;

  if (!SPIFFS.exists(CONFIG_FILE_PATH)) {
    Logger.log(F("✖ "));
    Logger.log(CONFIG_FILE_PATH);
    Logger.logln(F(" doesn't exist"));
    return false;
  }

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  if (!configFile) {
    Logger.logln(F("✖ Cannot open config file"));
    return false;
  }

  size_t configSize = configFile.size();

  if (configSize > MAX_JSON_CONFIG_FILE_BUFFER_SIZE) {
    Logger.logln(F("✖ Config file too big"));
    return false;
  }

  char buf[MAX_JSON_CONFIG_FILE_BUFFER_SIZE];
  configFile.readBytes(buf, configSize);
  configFile.close();

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& parsedJson = jsonBuffer.parseObject(buf);
  if (!parsedJson.success()) {
    Logger.logln(F("✖ Invalid JSON in the config file"));
    return false;
  }

  if (!Helpers::validateConfig(parsedJson)) {
    Logger.logln(F("✖ Config file is not valid"));
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
      Logger.logln(F("✖ Cannot open OTA file"));
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

const ConfigStruct& ConfigClass::get() {
  return this->_configStruct;
}

void ConfigClass::erase() {
  if (!this->_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);
  SPIFFS.remove(CONFIG_OTA_PATH);
}

void ConfigClass::write(const String& config) {
  if (!this->_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    Logger.logln(F("✖ Cannot open config file"));
    return;
  }

  configFile.print(config);
  configFile.close();
}

void ConfigClass::setOtaMode(bool enabled, const char* version) {
  if (!this->_spiffsBegin()) { return; }

  if (enabled) {
    File otaFile = SPIFFS.open(CONFIG_OTA_PATH, "w");
    if (!otaFile) {
      Logger.logln(F("✖ Cannot open OTA file"));
      return;
    }

    otaFile.print(version);
    otaFile.close();
  } else {
    SPIFFS.remove(CONFIG_OTA_PATH);
  }
}

const char* ConfigClass::getOtaVersion() {
  return this->_otaVersion;
}

BootMode ConfigClass::getBootMode() {
  return this->_bootMode;
}

void ConfigClass::log() {
  Logger.logln(F("{} Stored configuration:"));
  Logger.log(F("  • Hardware device ID: "));
  Logger.logln(Helpers::getDeviceId());
  Logger.log(F("  • Device ID: "));
  Logger.logln(this->_configStruct.deviceId);
  Logger.log(F("  • Boot mode: "));
  switch (this->_bootMode) {
    case BOOT_CONFIG:
      Logger.logln(F("configuration"));
      break;
    case BOOT_NORMAL:
      Logger.logln(F("normal"));
      break;
    case BOOT_OTA:
      Logger.logln(F("OTA"));
      break;
    default:
      Logger.logln(F("unknown"));
      break;
  }
  Logger.log(F("  • Name: "));
  Logger.logln(this->_configStruct.name);

  Logger.logln(F("  • Wi-Fi"));
  Logger.log(F("    ◦ SSID: "));
  Logger.logln(this->_configStruct.wifi.ssid);
  Logger.logln(F("    ◦ Password not shown"));

  Logger.logln(F("  • MQTT"));
  if (this->_configStruct.mqtt.server.mdns.enabled) {
    Logger.log(F("    ◦ mDNS: "));
    Logger.log(this->_configStruct.mqtt.server.mdns.service);
  } else {
    Logger.log(F("    ◦ Host: "));
    Logger.logln(this->_configStruct.mqtt.server.host);
    Logger.log(F("    ◦ Port: "));
    Logger.logln(this->_configStruct.mqtt.server.port);
  }
  Logger.log(F("    ◦ Base topic: "));
  Logger.logln(this->_configStruct.mqtt.baseTopic);
  Logger.log(F("    ◦ Auth? "));
  Logger.logln(this->_configStruct.mqtt.auth ? F("yes") : F("no"));
  if (this->_configStruct.mqtt.auth) {
    Logger.log(F("    ◦ Username: "));
    Logger.logln(this->_configStruct.mqtt.username);
    Logger.logln(F("    ◦ Password not shown"));
  }
  Logger.log(F("    ◦ SSL? "));
  Logger.logln(this->_configStruct.mqtt.server.ssl.enabled ? F("yes") : F("no"));
  if (this->_configStruct.mqtt.server.ssl.enabled) {
    Logger.log(F("    ◦ Fingerprint: "));
    if (strcmp_P(this->_configStruct.mqtt.server.ssl.fingerprint, PSTR("")) == 0) Logger.logln(F("unset"));
    else Logger.logln(this->_configStruct.mqtt.server.ssl.fingerprint);
  }

  Logger.logln(F("  • OTA"));
  Logger.log(F("    ◦ Enabled? "));
  Logger.logln(this->_configStruct.ota.enabled ? F("yes") : F("no"));
  if (this->_configStruct.ota.enabled) {
    if (this->_configStruct.ota.server.mdns.enabled) {
      Logger.log(F("    ◦ mDNS: "));
      Logger.log(this->_configStruct.ota.server.mdns.service);
    } else {
      Logger.log(F("    ◦ Host: "));
      Logger.logln(this->_configStruct.ota.server.host);
      Logger.log(F("    ◦ Port: "));
      Logger.logln(this->_configStruct.ota.server.port);
    }
    Logger.log(F("    ◦ Path: "));
    Logger.logln(this->_configStruct.ota.path);
    Logger.log(F("    ◦ SSL? "));
    Logger.logln(this->_configStruct.ota.server.ssl.enabled ? F("yes") : F("no"));
    if (this->_configStruct.ota.server.ssl.enabled) {
      Logger.log(F("    ◦ Fingerprint: "));
      if (strcmp_P(this->_configStruct.ota.server.ssl.fingerprint, PSTR("")) == 0) Logger.logln(F("unset"));
      else Logger.logln(this->_configStruct.ota.server.ssl.fingerprint);
    }
  }
}

ConfigClass HomieInternals::Config;

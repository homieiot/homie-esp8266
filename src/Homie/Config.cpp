#include "Config.hpp"

using namespace HomieInternals;

ConfigClass::ConfigClass()
: _spiffsBegan(false)
{
}

bool ConfigClass::_spiffsBegin() {
  if (!this->_spiffsBegan) {
    this->_spiffsBegan = SPIFFS.begin();
  }

  if (!this->_spiffsBegan) {
    Logger.logln("✖ Cannot mount filesystem");
  }

  return this->_spiffsBegan;
}

bool ConfigClass::load() {
  if (!this->_spiffsBegin()) { return false; }

  this->_bootMode = BOOT_CONFIG;

  if (!this->_spiffsBegan) {
    Logger.logln("✖ Cannot mount filesystem");
    return false;
  }

  if (!SPIFFS.exists(CONFIG_FILE_PATH)) {
    Logger.log(CONFIG_FILE_PATH);
    Logger.logln(" doesn't exist");
    return false;
  }

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
  if (!configFile) {
    Logger.logln("✖ Cannot open config file");
    return false;
  }

  size_t configSize = configFile.size();

  std::unique_ptr<char[]> buf(new char[configSize]);
  configFile.readBytes(buf.get(), configSize);

  StaticJsonBuffer<JSON_CONFIG_MAX_BUFFER_SIZE> jsonBuffer;
  JsonObject& parsedJson = jsonBuffer.parseObject(buf.get());
  if (!parsedJson.success()) {
    Logger.logln("✖ Invalid or too big config file");
    return false;
  }

  if (!Helpers.validateConfig(parsedJson)) {
    return false;
  }

  if (SPIFFS.exists(CONFIG_OTA_PATH)) {
    this->_bootMode = BOOT_OTA;
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

  this->_configStruct.name = strdup(reqName);
  this->_configStruct.wifi.ssid = strdup(reqWifiSsid);
  this->_configStruct.wifi.password = strdup(reqWifiPassword);
  this->_configStruct.mqtt.server.host = strdup(reqMqttHost);
  this->_configStruct.mqtt.server.port = reqMqttPort;
  this->_configStruct.mqtt.server.mdns.enabled = reqMqttMdns;
  this->_configStruct.mqtt.server.mdns.service = strdup(reqMqttMdnsService);
  this->_configStruct.mqtt.baseTopic = strdup(reqMqttBaseTopic);
  this->_configStruct.mqtt.auth = reqMqttAuth;
  this->_configStruct.mqtt.username = strdup(reqMqttUsername);
  this->_configStruct.mqtt.password = strdup(reqMqttPassword);
  this->_configStruct.mqtt.server.ssl.enabled = reqMqttSsl;
  this->_configStruct.mqtt.server.ssl.fingerprint = strdup(reqMqttFingerprint);
  this->_configStruct.ota.enabled = reqOtaEnabled;
  this->_configStruct.ota.server.host = strdup(reqOtaHost);
  this->_configStruct.ota.server.port = reqOtaPort;
  this->_configStruct.ota.server.mdns.enabled = reqOtaMdns;
  this->_configStruct.ota.server.mdns.service = strdup(reqOtaMdnsService);
  this->_configStruct.ota.path = strdup(reqOtaPath);
  this->_configStruct.ota.server.ssl.enabled = reqOtaSsl;
  this->_configStruct.ota.server.ssl.fingerprint = strdup(reqOtaFingerprint);

  return true;
}

ConfigStruct& ConfigClass::get() {
  return this->_configStruct;
}

void ConfigClass::erase() {
  if (!this->_spiffsBegin()) { return; }

  if (!this->_spiffsBegan) {
    Logger.logln("✖ Cannot mount filesystem");
    return;
  }

  SPIFFS.remove(CONFIG_FILE_PATH);
  SPIFFS.remove(CONFIG_OTA_PATH);
}

void ConfigClass::write(const String& config) {
  if (!this->_spiffsBegin()) { return; }

  SPIFFS.remove(CONFIG_FILE_PATH);

  File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
  if (!configFile) {
    Logger.logln("✖ Cannot open config file");
    return;
  }

  configFile.print(config);
  configFile.close();
}

void ConfigClass::setOtaMode(bool enabled) {
  if (!this->_spiffsBegin()) { return; }

  if (enabled) {
    File otaFile = SPIFFS.open(CONFIG_OTA_PATH, "w");
    if (!otaFile) {
      Logger.logln("✖ Cannot open OTA file");
      return;
    }

    otaFile.print(1);
    otaFile.close();
  } else {
    SPIFFS.remove(CONFIG_OTA_PATH);
  }
}

BootMode ConfigClass::getBootMode() {
  return this->_bootMode;
}

void ConfigClass::log() {
  Logger.logln("⚙ Stored configuration:");
  Logger.log("  • Device ID: ");
  Logger.logln(Helpers.getDeviceId());
  Logger.log("  • Boot mode: ");
  switch (this->_bootMode) {
    case BOOT_CONFIG:
      Logger.logln("configuration");
      break;
    case BOOT_NORMAL:
      Logger.logln("normal");
      break;
    case BOOT_OTA:
      Logger.logln("OTA");
      break;
    default:
      Logger.logln("unknown");
      break;
  }
  Logger.log("  • Name: ");
  Logger.logln(this->_configStruct.name);

  Logger.logln("  • Wi-Fi");
  Logger.log("    • SSID: ");
  Logger.logln(this->_configStruct.wifi.ssid);
  Logger.logln("    • Password not shown");

  Logger.logln("  • MQTT");
  if (this->_configStruct.mqtt.server.mdns.enabled) {
    Logger.log("    • mDNS: ");
    Logger.log(this->_configStruct.mqtt.server.mdns.service);
  } else {
    Logger.log("    • Host: ");
    Logger.logln(this->_configStruct.mqtt.server.host);
    Logger.log("    • Port: ");
    Logger.logln(String(this->_configStruct.mqtt.server.port));
  }
  Logger.log("    • Base topic: ");
  Logger.logln(this->_configStruct.mqtt.baseTopic);
  Logger.log("    • Auth: ");
  Logger.logln(this->_configStruct.mqtt.auth ? "yes" : "no");
  if (this->_configStruct.mqtt.auth) {
    Logger.log("    • Username: ");
    Logger.logln(this->_configStruct.mqtt.username);
    Logger.logln("    • Password not shown");
  }
  Logger.log("    • SSL: ");
  Logger.logln(this->_configStruct.mqtt.server.ssl.enabled ? "yes" : "no");
  if (this->_configStruct.mqtt.server.ssl.enabled) {
    Logger.log("    • Fingerprint: ");
    Logger.logln(this->_configStruct.mqtt.server.ssl.fingerprint);
  }

  Logger.logln("  • OTA");
  Logger.log("    • Enabled: ");
  Logger.logln(this->_configStruct.ota.enabled ? "yes" : "no");
  if (this->_configStruct.ota.enabled) {
    if (this->_configStruct.ota.server.mdns.enabled) {
      Logger.log("    • mDNS: ");
      Logger.log(this->_configStruct.ota.server.mdns.service);
    } else {
      Logger.log("    • Host: ");
      Logger.logln(this->_configStruct.ota.server.host);
      Logger.log("    • Port: ");
      Logger.logln(String(this->_configStruct.ota.server.port));
    }
    Logger.log("    • Path: ");
    Logger.logln(String(this->_configStruct.ota.path));
    Logger.log("    • SSL: ");
    Logger.logln(this->_configStruct.ota.server.ssl.enabled ? "yes" : "no");
    if (this->_configStruct.ota.server.ssl.enabled) {
      Logger.log("    • Fingerprint: ");
      Logger.logln(this->_configStruct.ota.server.ssl.fingerprint);
    }
  }
}

ConfigClass HomieInternals::Config;

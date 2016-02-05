#include "Config.hpp"

using namespace HomieInternals;

ConfigClass::ConfigClass()
: _spiffs_began(false)
{
}

bool ConfigClass::_spiffsBegin() {
  if (!this->_spiffs_began) {
    this->_spiffs_began = SPIFFS.begin();
    return this->_spiffs_began;
  }
}

bool ConfigClass::load() {
  this->_spiffsBegin();

  this->_boot_mode = BOOT_CONFIG;

  if (!this->_spiffs_began) {
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
  JsonObject& parsed_json = jsonBuffer.parseObject(buf.get());
  if (!parsed_json.success()) {
    Logger.logln("✖ Invalid or too big config file");
    return false;
  }

  if (!Helpers::validateConfig(parsed_json)) {
    return false;
  }

  if (SPIFFS.exists(CONFIG_OTA_PATH)) {
    this->_boot_mode = BOOT_OTA;
  } else {
    this->_boot_mode = BOOT_NORMAL;
  }

  const char* req_name = parsed_json["name"];
  const char* req_wifi_ssid = parsed_json["wifi"]["ssid"];
  const char* req_wifi_password = parsed_json["wifi"]["password"];
  const char* req_mqtt_host = parsed_json["mqtt"]["host"];
  uint16_t req_mqtt_port = DEFAULT_MQTT_PORT;
  if (parsed_json["mqtt"].as<JsonObject&>().containsKey("port")) {
    req_mqtt_port = parsed_json["mqtt"]["port"];
  }
  bool req_mqtt_auth = false;
  if (parsed_json["mqtt"].as<JsonObject&>().containsKey("auth")) {
    req_mqtt_auth = parsed_json["mqtt"]["auth"];
  }
  const char* req_mqtt_username = "";
  if (parsed_json["mqtt"].as<JsonObject&>().containsKey("username")) {
    req_mqtt_username = parsed_json["mqtt"]["username"];
  }
  const char* req_mqtt_password = "";
  if (parsed_json["mqtt"].as<JsonObject&>().containsKey("password")) {
    req_mqtt_password = parsed_json["mqtt"]["password"];
  }
  bool req_mqtt_ssl = false;
  if (parsed_json["mqtt"].as<JsonObject&>().containsKey("ssl")) {
    req_mqtt_ssl = parsed_json["mqtt"]["ssl"];
  }
  const char* req_mqtt_fingerprint = "";
  if (parsed_json["mqtt"].as<JsonObject&>().containsKey("fingerprint")) {
    req_mqtt_fingerprint = parsed_json["mqtt"]["fingerprint"];
  }
  bool req_ota_enabled = false;
  if (parsed_json["ota"].as<JsonObject&>().containsKey("enabled")) {
    req_ota_enabled = parsed_json["ota"]["enabled"];
  }
  const char* req_ota_host = req_mqtt_host;
  if (parsed_json["ota"].as<JsonObject&>().containsKey("host")) {
    req_ota_host = parsed_json["ota"]["host"];
  }
  uint16_t req_ota_port = DEFAULT_OTA_PORT;
  if (parsed_json["ota"].as<JsonObject&>().containsKey("port")) {
    req_ota_port = parsed_json["ota"]["port"];
  }
  const char* req_ota_path = DEFAULT_OTA_PATH;
  if (parsed_json["ota"].as<JsonObject&>().containsKey("path")) {
    req_ota_path = parsed_json["ota"]["path"];
  }
  bool req_ota_ssl = false;
  if (parsed_json["ota"].as<JsonObject&>().containsKey("ssl")) {
    req_ota_ssl = parsed_json["ota"]["ssl"];
  }
  const char* req_ota_fingerprint = "";
  if (parsed_json["ota"].as<JsonObject&>().containsKey("fingerprint")) {
    req_ota_fingerprint = parsed_json["ota"]["fingerprint"];
  }

  strcpy(this->_config_struct.name, req_name);
  strcpy(this->_config_struct.wifi.ssid, req_wifi_ssid);
  strcpy(this->_config_struct.wifi.password, req_wifi_password);
  strcpy(this->_config_struct.mqtt.host, req_mqtt_host);
  this->_config_struct.mqtt.port = req_mqtt_port;
  this->_config_struct.mqtt.auth = req_mqtt_auth;
  strcpy(this->_config_struct.mqtt.username, req_mqtt_username);
  strcpy(this->_config_struct.mqtt.password, req_mqtt_password);
  this->_config_struct.mqtt.ssl = req_mqtt_ssl;
  strcpy(this->_config_struct.mqtt.fingerprint, req_mqtt_fingerprint);
  this->_config_struct.ota.enabled = req_ota_enabled;
  strcpy(this->_config_struct.ota.host, req_ota_host);
  this->_config_struct.ota.port = req_ota_port;
  strcpy(this->_config_struct.ota.path, req_ota_path);
  this->_config_struct.ota.ssl = req_ota_ssl;
  strcpy(this->_config_struct.ota.fingerprint, req_ota_fingerprint);

  return true;
}

ConfigStruct& ConfigClass::get() {
  return this->_config_struct;
}

void ConfigClass::erase() {
  this->_spiffsBegin();

  if (!this->_spiffs_began) {
    Logger.logln("✖ Cannot mount filesystem");
    return;
  }

  SPIFFS.remove(CONFIG_FILE_PATH);
  SPIFFS.remove(CONFIG_OTA_PATH);
}

void ConfigClass::write(const String& config) {
  this->_spiffsBegin();

  if (!this->_spiffs_began) {
    Logger.logln("✖ Cannot mount filesystem");
    return;
  }

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
  this->_spiffsBegin();

  if (!this->_spiffs_began) {
    Logger.logln("✖ Cannot mount filesystem");
    return;
  }

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
  return this->_boot_mode;
}

void ConfigClass::log() {
  Logger.logln("⚙ Stored configuration:");
  Logger.log("  • Device ID: ");
  Logger.logln(Helpers::getDeviceId());
  Logger.log("  • Boot mode: ");
  switch (this->_boot_mode) {
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
  Logger.logln(this->_config_struct.name);

  Logger.logln("  • Wi-Fi");
  Logger.log("    • SSID: ");
  Logger.logln(this->_config_struct.wifi.ssid);
  Logger.logln("    • Password not shown");

  Logger.logln("  • MQTT");
  Logger.log("    • Host: ");
  Logger.log(this->_config_struct.mqtt.host);
  Logger.logln(" "); // Needed because else ??? in serial monitor
  Logger.log("    • Port: ");
  Logger.logln(String(this->_config_struct.mqtt.port));
  Logger.log("    • Auth: ");
  Logger.logln(this->_config_struct.mqtt.auth ? "yes" : "no");
  if (this->_config_struct.mqtt.auth) {
    Logger.log("    • Username: ");
    Logger.logln(this->_config_struct.mqtt.username);
    Logger.logln("    • Password not shown");
  }
  Logger.log("    • SSL: ");
  Logger.logln(this->_config_struct.mqtt.ssl ? "yes" : "no");
  if (this->_config_struct.mqtt.ssl) {
    Logger.log("    • Fingerprint: ");
    Logger.logln(this->_config_struct.mqtt.fingerprint);
  }

  Logger.logln("  • OTA");
  Logger.log("    • Enabled: ");
  Logger.logln(this->_config_struct.ota.enabled ? "yes" : "no");
  if (this->_config_struct.ota.enabled) {
    Logger.log("    • Host: ");
    Logger.logln(this->_config_struct.ota.host);
    Logger.log("    • Port: ");
    Logger.logln(String(this->_config_struct.ota.port));
    Logger.log("    • Path: ");
    Logger.logln(String(this->_config_struct.ota.path));
    Logger.log("    • SSL: ");
    Logger.logln(this->_config_struct.ota.ssl ? "yes" : "no");
    if (this->_config_struct.mqtt.ssl) {
      Logger.log("    • Fingerprint: ");
      Logger.logln(this->_config_struct.ota.fingerprint);
    }
  }
}

ConfigClass HomieInternals::Config;

#include "BootConfig.hpp"

using namespace HomieInternals;

BootConfig::BootConfig()
: Boot("config")
, _http(80)
, _ssid_count(0)
, _last_wifi_scan(0)
, _last_wifi_scan_ended(true)
, _flagged_for_reboot(false)
, _flagged_for_reboot_at(0)
{
}

BootConfig::~BootConfig() {
}

void BootConfig::setup() {
  Boot::setup();

  digitalWrite(BUILTIN_LED, LOW);

  String device_id = Helpers::getDeviceId();

  Logger.log("Device ID is ");
  Logger.logln(device_id);

  WiFi.mode(WIFI_AP);

  IPAddress ap_ip(192, 168, 1, 1);
  String ap_name = String("Homie-");
  ap_name += device_id;

  WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ap_name.c_str(), device_id.c_str());

  Logger.log("AP started as ");
  Logger.logln(ap_name);

  // Trigger sync Wi-Fi scan (don't do before AP init or doesn't work)
  this->_ssid_count = WiFi.scanNetworks();
  this->_last_wifi_scan = millis();
  this->_json_wifi_networks = this->_generateNetworksJson();

  this->_dns.setTTL(300);
  this->_dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
  this->_dns.start(53, "homie.config", ap_ip);

  this->_http.on("/heart", HTTP_GET, [this]() {
    Logger.logln("Received heart request");
    this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"heart\":\"beat\"}");
  });
  this->_http.on("/networks", HTTP_GET, std::bind(&BootConfig::_onNetworksRequest, this));
  this->_http.on("/config", HTTP_PUT, std::bind(&BootConfig::_onConfigRequest, this));
  this->_http.on("/config", HTTP_OPTIONS, [this]() { // CORS
    Logger.logln("Received CORS request for /config");
    this->_http.sendContent(FPSTR(PROGMEM_CONFIG_CORS));
  });
  this->_http.begin();
}

String BootConfig::_generateNetworksJson() {
  DynamicJsonBuffer generatedJsonBuffer;
  JsonObject& json = generatedJsonBuffer.createObject();

  JsonArray& networks = json.createNestedArray("networks");
  for (int network = 0; network < this->_ssid_count; network++) {
    JsonObject& json_network = generatedJsonBuffer.createObject();
    json_network["ssid"] = WiFi.SSID(network);
    json_network["rssi"] = WiFi.RSSI(network);
    switch (WiFi.encryptionType(network)) {
      case ENC_TYPE_WEP:
        json_network["encryption"] = "wep";
        break;
      case ENC_TYPE_TKIP:
        json_network["encryption"] = "wpa";
        break;
      case ENC_TYPE_CCMP:
        json_network["encryption"] = "wpa2";
        break;
      case ENC_TYPE_NONE:
        json_network["encryption"] = "none";
        break;
      case ENC_TYPE_AUTO:
        json_network["encryption"] = "auto";
        break;
    }

    networks.add(json_network);
  }

  // 15 bytes: {"networks":[]}
  // 75 bytes: {"ssid":"thisisa32characterlongstringyes!","rssi":-99,"encryption":"none"}, (-1 for leading ","), +1 for terminator
  char json_string[15 + (75 * this->_ssid_count) - 1 + 1];
  size_t json_length = json.printTo(json_string, sizeof(json_string));
  return String(json_string);
}

void BootConfig::_onNetworksRequest() {
  Logger.logln("Received networks request");
  this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), this->_json_wifi_networks);
}

void BootConfig::_onConfigRequest() {
  Logger.logln("Received config request");
  if (this->_flagged_for_reboot) {
    Logger.logln("✖ Device already configured");
    this->_http.send(403, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  StaticJsonBuffer<JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(6)> parseJsonBuffer; // Max 4 elements at root, 2 elements in nested, etc...
  JsonObject& parsed_json = parseJsonBuffer.parseObject((char*)this->_http.arg("plain").c_str());
  if (!parsed_json.success()) {
    Logger.logln("✖ Invalid or too big JSON");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  if (!Helpers::validateConfig(parsed_json)) {
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
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

  Config.get().configured = true;
  Config.get().boot_mode = BOOT_NORMAL;
  strcpy(Config.get().name, req_name);
  strcpy(Config.get().wifi.ssid, req_wifi_ssid);
  strcpy(Config.get().wifi.password, req_wifi_password);
  strcpy(Config.get().mqtt.host, req_mqtt_host);
  Config.get().mqtt.port = req_mqtt_port;
  Config.get().mqtt.auth = req_mqtt_auth;
  strcpy(Config.get().mqtt.username, req_mqtt_username);
  strcpy(Config.get().mqtt.password, req_mqtt_password);
  Config.get().mqtt.ssl = req_mqtt_ssl;
  strcpy(Config.get().mqtt.fingerprint, req_mqtt_fingerprint);
  Config.get().ota.enabled = req_ota_enabled;
  strcpy(Config.get().ota.host, req_ota_host);
  Config.get().ota.port = req_ota_port;
  strcpy(Config.get().ota.path, req_ota_path);
  Config.get().ota.ssl = req_ota_ssl;
  strcpy(Config.get().ota.fingerprint, req_ota_fingerprint);
  Config.save();
  Config.log();

  Logger.logln("✔ Configured");

  this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"success\":true}");

  this->_flagged_for_reboot = true; // We don't reboot immediately, otherwise the response above is not sent
  this->_flagged_for_reboot_at = millis();
}

void BootConfig::loop() {
  Boot::loop();

  this->_dns.processNextRequest();
  this->_http.handleClient();

  if (this->_flagged_for_reboot) {
    if (millis() - this->_flagged_for_reboot_at >= 3000UL) {
      Logger.logln("↻ Rebooting in normal mode");
      ESP.restart();
    }

    return;
  }

  if (!this->_last_wifi_scan_ended) {
    int8_t scan_result = WiFi.scanComplete();

    switch (scan_result) {
      case WIFI_SCAN_RUNNING:
        return;
      case WIFI_SCAN_FAILED:
        Logger.logln("✖ Wi-Fi scan failed");
        this->_ssid_count = 0;
        break;
      default:
        Logger.logln("✔ Wi-Fi scan completed");
        this->_ssid_count = scan_result;
        this->_json_wifi_networks = this->_generateNetworksJson();
        break;
    }

    this->_last_wifi_scan_ended = true;
  }

  unsigned long now = millis();
  if (now - this->_last_wifi_scan >= 20000UL && this->_last_wifi_scan_ended) {
    Logger.logln("Triggering Wi-Fi scan");
    WiFi.scanNetworks(true);
    this->_last_wifi_scan = now;
    this->_last_wifi_scan_ended = false;
  }
}

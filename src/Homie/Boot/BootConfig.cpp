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

  char chip_id[6 + 1];
  sprintf(chip_id, "%06x", ESP.getChipId());
  char flash_chip_id[6 + 1];
  sprintf(flash_chip_id, "%06x", ESP.getFlashChipId());

  String truncated_flash_id = String(flash_chip_id);
  truncated_flash_id = truncated_flash_id.substring(4);

  String device_id = String(chip_id);
  device_id += truncated_flash_id;

  Logger.log("Device ID is ");
  Logger.logln(device_id);

  String tmp_hostname = String("Homie-");
  tmp_hostname += device_id;

  WiFi.hostname(tmp_hostname);

  digitalWrite(BUILTIN_LED, LOW);

  WiFi.mode(WIFI_AP);

  IPAddress apIP(192, 168, 1, 1);

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(tmp_hostname.c_str(), device_id.c_str());

  Logger.log("AP started as ");
  Logger.logln(tmp_hostname);

  // Trigger sync Wi-Fi scan (don't do before AP init or doesn't work)
  this->_ssid_count = WiFi.scanNetworks();
  this->_last_wifi_scan = millis();
  this->_json_wifi_networks = this->_generateNetworksJson();

  this->_dns.setTTL(300);
  this->_dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
  this->_dns.start(53, "homie.config", apIP);

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

  StaticJsonBuffer<JSON_OBJECT_SIZE(7)> parseJsonBuffer; // Max seven elements in object
  JsonObject& parsed_json = parseJsonBuffer.parseObject((char*)this->_http.arg("plain").c_str());
  if (!parsed_json.success()) {
    Logger.logln("✖ Invalid or too big JSON");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  if (!parsed_json.containsKey("name") || !parsed_json["name"].is<const char*>()) {
    Logger.logln("✖ name is not a string");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (!parsed_json.containsKey("wifi_ssid") || !parsed_json["wifi_ssid"].is<const char*>()) {
    Logger.logln("✖ wifi_ssid is not a string");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (!parsed_json.containsKey("wifi_password") || !parsed_json["wifi_password"].is<const char*>()) {
    Logger.logln("✖ wifi_password is not a string");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (!parsed_json.containsKey("homie_host") || !parsed_json["homie_host"].is<const char*>()) {
    Logger.logln("✖ homie_host is not a string");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (parsed_json.containsKey("homie_port") && !parsed_json["homie_port"].is<uint16_t>()) {
    Logger.logln("✖ homie_port is not an unsigned integer");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (parsed_json.containsKey("homie_ota_path") && !parsed_json["homie_ota_path"].is<const char*>()) {
    Logger.logln("✖ homie_ota_path is not a string");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (parsed_json.containsKey("homie_ota_port") && !parsed_json["homie_ota_port"].is<uint16_t>()) {
    Logger.logln("✖ homie_ota_port is not an unsigned integer");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  const char* req_name = parsed_json["name"];
  const char* req_wifi_ssid = parsed_json["wifi_ssid"];
  const char* req_wifi_password = parsed_json["wifi_password"];
  const char* req_homie_host = parsed_json["homie_host"];
  uint16_t req_homie_port = DEFAULT_HOMIE_PORT;
  if (parsed_json.containsKey("homie_port")) {
    req_homie_port = parsed_json["homie_port"].as<uint16_t>();
  }
  const char* req_homie_ota_path = DEFAULT_HOMIE_OTA_PATH;
  if (parsed_json.containsKey("homie_ota_path")) {
    req_homie_ota_path = parsed_json["homie_ota_path"];
  }
  uint16_t req_homie_ota_port = DEFAULT_HOMIE_OTA_PORT;
  if (parsed_json.containsKey("homie_ota_port")) {
    req_homie_ota_port = parsed_json["homie_ota_port"].as<uint16_t>();
  }

  if (strcmp(req_name, "") == 0) {
    Logger.logln("✖ name is empty");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (strcmp(req_wifi_ssid, "") == 0) {
    Logger.logln("✖ wifi_ssid is empty");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }
  if (strcmp(req_homie_host, "") == 0) {
    Logger.logln("✖ homie_host is empty");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  // Check if hostname only [a-z0-9\-]
  for (int i = 0; i < strlen(req_name); i++){
    if (!((req_name[i] >= 'a' && req_name[i] <= 'z') || (req_name[i] >= '0' && req_name[i] <= '9') || req_name[i] == '-')) {
      Logger.logln("✖ name contains unauthorized characters");
      this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
      return;
    }
  }
  // Check if hostname doesn't start or end with '-'
  if (req_name[0] == '-' || req_name[strlen(req_name) - 1] == '-') {
    Logger.logln("✖ name starts or ends with a dash");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  Config.hostname = req_name;
  Config.wifi_ssid = req_wifi_ssid;
  Config.wifi_password = req_wifi_password;
  Config.homie_host = req_homie_host;
  Config.homie_port = req_homie_port;
  Config.homie_ota_path = req_homie_ota_path;
  Config.homie_ota_port = req_homie_ota_port;
  Config.boot_mode = BOOT_NORMAL;
  Config.configured = true;
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
    if (millis() - this->_flagged_for_reboot_at >= 5000UL) {
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

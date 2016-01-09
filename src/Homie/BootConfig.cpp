#include "BootConfig.hpp"

BootConfig::BootConfig()
: Boot("config")
, _http(80)
{
}

BootConfig::~BootConfig() {
}

void BootConfig::setup() {
  Boot::setup();

  String temp_hostname = String("homie-");
  temp_hostname += ESP.getChipId();

  WiFi.hostname(temp_hostname);

  digitalWrite(BUILTIN_LED, LOW);

  String ap_ssid = String("Homie-");
  ap_ssid += ESP.getChipId();

  WiFi.mode(WIFI_AP);

  IPAddress apIP(192, 168, 1, 1);

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ap_ssid.c_str());

  this->_dns.setTTL(300);
  this->_dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
  this->_dns.start(53, "www.homieconfigurator.com", apIP);

  this->_http.on("/networks", HTTP_GET, std::bind(&BootConfig::_onNetworksRequest, this));
  this->_http.on("/config", HTTP_PUT, std::bind(&BootConfig::_onConfigRequest, this));
  this->_http.on("/config", HTTP_OPTIONS, [this]() { // CORS
    String cors;
    cors += FPSTR(CORS);
    this->_http.sendContent(cors);
  });
  this->_http.begin();
}

void BootConfig::_onNetworksRequest() {
  DynamicJsonBuffer generatedJsonBuffer;
  JsonObject& json = generatedJsonBuffer.createObject();

  byte ssid_count = WiFi.scanNetworks();

  JsonArray& networks = json.createNestedArray("networks");
  for (int network = 0; network < ssid_count; network++) {
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

  // 75 bytes: {"ssid":"thisisa32characterlongstringyes!","rssi":-99,"encryption":"none"}, (-1 for leading ",")
  char json_string[(75 * ssid_count) - 1];
  size_t json_length = json.printTo(json_string, sizeof(json_string));
  String json_arduino_string = String(json_string);
  this->_http.send(200, "application/json", json_arduino_string);
}

void BootConfig::_onConfigRequest() {
  StaticJsonBuffer<JSON_OBJECT_SIZE(4)> parseJsonBuffer; // Max four elements in object
  JsonObject& parsed_json = parseJsonBuffer.parseObject((char*)this->_http.arg("plain").c_str());
  if (!parsed_json.success()) { Serial.println("Invalid or too big JSON"); return; }

  if (!parsed_json.containsKey("name") || !parsed_json["name"].is<const char*>()) { Serial.println("Name is not a string"); return; }
  if (!parsed_json.containsKey("wifi_ssid") || !parsed_json["wifi_ssid"].is<const char*>()) { Serial.println("Wi-Fi SSID is not a string"); return; }
  if (!parsed_json.containsKey("wifi_password") || !parsed_json["wifi_password"].is<const char*>()) { Serial.println("Wi-Fi password is not a string"); return; }
  if (!parsed_json.containsKey("homie_host") || !parsed_json["homie_host"].is<const char*>()) { Serial.println("Homie host is not a string"); return; }

  const char* req_name = parsed_json["name"];
  const char* req_wifi_ssid = parsed_json["wifi_ssid"];
  const char* req_wifi_password = parsed_json["wifi_password"];
  const char* req_homie_host = parsed_json["homie_host"];

  if (strcmp(req_name, "") == 0) { Serial.println("Name is empty"); return; }
  if (strcmp(req_wifi_ssid, "") == 0) { Serial.println("Wi-Fi SSID is empty"); return; }
  if (strcmp(req_homie_host, "") == 0) { Serial.println("Homie host is empty"); return; }

  // Check if hostname only [a-z0-9\-]
  for (int i = 0; i < strlen(req_name); i++){
    if (!((req_name[i] >= 'a' && req_name[i] <= 'z') || (req_name[i] >= '0' && req_name[i] <= '9') || req_name[i] == '-')) {
      Serial.println("Name contains unauthorized characters");
      return;
    }
  }
  // Check if hostname doesn't start or end with '-'
  if (req_name[0] == '-' || req_name[strlen(req_name) - 1] == '-') { Serial.println("Name is starts or ends with a dash"); return; }

  Config.hostname = req_name;
  Config.wifi_ssid = req_wifi_ssid;
  Config.wifi_password = req_wifi_password;
  Config.homie_host = req_homie_host;
  Config.boot_mode = BOOT_NORMAL;
  Config.configured = true;
  Config.save();

  Serial.println("Configured");

  this->_http.send(200, "application/json", "{\"success\":true}");

  delay(1000); // Might help for the network stack to send whole HTTP response

  ESP.restart();
}

void BootConfig::loop() {
  Boot::loop();

  this->_dns.processNextRequest();
  this->_http.handleClient();
}

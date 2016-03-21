#include "BootConfig.hpp"

using namespace HomieInternals;

BootConfig::BootConfig(Interface* interface)
: Boot(interface, "config")
, _interface(interface)
, _http(80)
, _ssidCount(0)
, _wifiScanAvailable(false)
, _lastWifiScan(0)
, _lastWifiScanEnded(true)
, _flaggedForReboot(false)
, _flaggedForRebootAt(0)
{
}

BootConfig::~BootConfig() {
}

void BootConfig::setup() {
  Boot::setup();

  if (this->_interface->led.enable) {
    digitalWrite(this->_interface->led.pin, this->_interface->led.on);
  }

  const char* device_id = Helpers.getDeviceId();

  Logger.log("Device ID is ");
  Logger.logln(device_id);

  WiFi.mode(WIFI_AP);

  IPAddress ap_ip(192, 168, 1, 1);
  char ap_name[MAX_LENGTH_WIFI_SSID] = "";
  strcat(ap_name, this->_interface->brand);
  strcat(ap_name, "-");
  strcat(ap_name, Helpers.getDeviceId());

  WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ap_name, device_id);

  Logger.log("AP started as ");
  Logger.logln(ap_name);

  this->_dns.setTTL(300);
  this->_dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
  this->_dns.start(53, "homie.config", ap_ip);

  this->_http.on("/", HTTP_GET, [this]() {
    Logger.logln("Received index request");
    this->_http.send(200, "text/plain", "See Configuration API usage: https://github.com/marvinroger/homie-esp8266/wiki/6.-Configuration-API");
  });
  this->_http.on("/heart", HTTP_GET, [this]() {
    Logger.logln("Received heart request");
    this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"heart\":\"beat\"}");
  });
  this->_http.on("/device-info", HTTP_GET, std::bind(&BootConfig::_onDeviceInfoRequest, this));
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
  for (int network = 0; network < this->_ssidCount; network++) {
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
  String json_string;
  json_string.reserve(15 + (75 * this->_ssidCount) - 1 + 1);
  json.printTo(json_string);
  return json_string;
}

void BootConfig::_onDeviceInfoRequest() {
  Logger.logln("Received device info request");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["device_id"] = Helpers.getDeviceId();
  json["homie_version"] = VERSION;
  JsonObject& firmware = json.createNestedObject("firmware");
  firmware["name"] = this->_interface->firmware.name;
  firmware["version"] = this->_interface->firmware.version;

  JsonArray& nodes = json.createNestedArray("nodes");
  for (int i = 0; i < this->_interface->registeredNodes.size(); i++) {
    HomieNode node = this->_interface->registeredNodes[i];
    JsonObject& json_node = jsonBuffer.createObject();
    json_node["id"] = node.id;
    json_node["type"] = node.type;

    nodes.add(json_node);
  }

  // 110 bytes for {"homie_version":"11.10.0","firmware":{"name":"awesome-light-great-top","version":"11.10.0-beta"},"nodes":[]}
  // 60 bytes for {"id":"lightifydefoulooooo","type":"lightifydefouloooo"}, (-1 for leading ","), +1 for terminator
  String jsonString;
  jsonString.reserve(110 + (60 * this->_interface->registeredNodes.size()) - 1 + 1);
  json.printTo(jsonString);
  this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), jsonString);
}

void BootConfig::_onNetworksRequest() {
  Logger.logln("Received networks request");
  if (this->_wifiScanAvailable) {
    this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), this->_jsonWifiNetworks);
  } else {
    this->_http.send(503, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_NETWORKS_FAILURE));
  }
}

void BootConfig::_onConfigRequest() {
  Logger.logln("Received config request");
  if (this->_flaggedForReboot) {
    Logger.logln("✖ Device already configured");
    this->_http.send(403, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  StaticJsonBuffer<JSON_CONFIG_MAX_BUFFER_SIZE> parseJsonBuffer;
  JsonObject& parsed_json = parseJsonBuffer.parseObject((char*)this->_http.arg("plain").c_str());
  if (!parsed_json.success()) {
    Logger.logln("✖ Invalid or too big JSON");
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  if (!Helpers.validateConfig(parsed_json)) {
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_FAILURE));
    return;
  }

  Config.write(this->_http.arg("plain"));

  Logger.logln("✔ Configured");

  this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"success\":true}");

  this->_flaggedForReboot = true; // We don't reboot immediately, otherwise the response above is not sent
  this->_flaggedForRebootAt = millis();
}

void BootConfig::loop() {
  Boot::loop();

  this->_dns.processNextRequest();
  this->_http.handleClient();

  if (this->_flaggedForReboot) {
    if (millis() - this->_flaggedForRebootAt >= 3000UL) {
      Logger.logln("↻ Rebooting in normal mode");
      ESP.restart();
    }

    return;
  }

  if (!this->_lastWifiScanEnded) {
    int8_t scan_result = WiFi.scanComplete();

    switch (scan_result) {
      case WIFI_SCAN_RUNNING:
        return;
      case WIFI_SCAN_FAILED:
        Logger.logln("✖ Wi-Fi scan failed");
        this->_ssidCount = 0;
        this->_lastWifiScan = 0;
        break;
      default:
        Logger.logln("✔ Wi-Fi scan completed");
        this->_ssidCount = scan_result;
        this->_jsonWifiNetworks = this->_generateNetworksJson();
        this->_wifiScanAvailable = true;
        break;
    }

    this->_lastWifiScanEnded = true;
  }

  unsigned long now = millis();
  if ((now - this->_lastWifiScan >= CONFIG_SCAN_INTERVAL || this->_lastWifiScan == 0) && this->_lastWifiScanEnded) {
    Logger.logln("Triggering Wi-Fi scan");
    WiFi.scanNetworks(true);
    this->_lastWifiScan = now;
    this->_lastWifiScanEnded = false;
  }
}

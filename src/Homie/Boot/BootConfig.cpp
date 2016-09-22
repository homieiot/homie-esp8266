#include "BootConfig.hpp"

using namespace HomieInternals;

BootConfig::BootConfig()
: Boot("config")
, _http(80)
, _ssidCount(0)
, _wifiScanAvailable(false)
, _lastWifiScanEnded(true)
, _jsonWifiNetworks()
, _flaggedForReboot(false)
, _flaggedForRebootAt(0)
{
  this->_wifiScanTimer.setInterval(CONFIG_SCAN_INTERVAL);
}

BootConfig::~BootConfig() {
}

void BootConfig::setup() {
  Boot::setup();

  if (this->_interface->led.enabled) {
    digitalWrite(this->_interface->led.pin, this->_interface->led.on);
  }

  const char* deviceId = Helpers::getDeviceId();

  this->_interface->logger->log(F("Device ID is "));
  this->_interface->logger->logln(deviceId);

  WiFi.mode(WIFI_AP);

  char apName[MAX_WIFI_SSID_LENGTH];
  strcpy(apName, this->_interface->brand);
  strcat_P(apName, PSTR("-"));
  strcat(apName, Helpers::getDeviceId());

  WiFi.softAPConfig(ACCESS_POINT_IP, ACCESS_POINT_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, deviceId);

  this->_interface->logger->log(F("AP started as "));
  this->_interface->logger->logln(apName);

  this->_dns.setTTL(300);
  this->_dns.setErrorReplyCode(DNSReplyCode::ServerFailure);
  this->_dns.start(53, F("homie.config"), ACCESS_POINT_IP);

  this->_http.on("/", HTTP_GET, [this]() {
    this->_interface->logger->logln(F("Received index request"));
    this->_http.send(200, F("text/plain"), F("See Configuration API usage: http://marvinroger.viewdocs.io/homie-esp8266/6.-Configuration-API"));
  });
  this->_http.on("/heart", HTTP_GET, [this]() {
    this->_interface->logger->logln(F("Received heart request"));
    this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), F("{\"heart\":\"beat\"}"));
  });
  this->_http.on("/device-info", HTTP_GET, std::bind(&BootConfig::_onDeviceInfoRequest, this));
  this->_http.on("/networks", HTTP_GET, std::bind(&BootConfig::_onNetworksRequest, this));
  this->_http.on("/config", HTTP_PUT, std::bind(&BootConfig::_onConfigRequest, this));
  this->_http.on("/config", HTTP_OPTIONS, [this]() { // CORS
    this->_interface->logger->logln(F("Received CORS request for /config"));
    this->_http.sendContent(FPSTR(PROGMEM_CONFIG_CORS));
  });
  this->_http.begin();
}

void BootConfig::_generateNetworksJson() {
  DynamicJsonBuffer generatedJsonBuffer = DynamicJsonBuffer(JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(this->_ssidCount) + (this->_ssidCount * JSON_OBJECT_SIZE(3))); // 1 at root, 3 in childrend
  JsonObject& json = generatedJsonBuffer.createObject();

  int jsonLength = 15; // {"networks":[]}
  JsonArray& networks = json.createNestedArray("networks");
  for (int network = 0; network < this->_ssidCount; network++) {
    jsonLength += 36; // {"ssid":"","rssi":,"encryption":""},
    JsonObject& jsonNetwork = generatedJsonBuffer.createObject();
    jsonLength += WiFi.SSID(network).length();
    jsonNetwork["ssid"] = WiFi.SSID(network);
    jsonLength += 4;
    jsonNetwork["rssi"] = WiFi.RSSI(network);
    jsonLength += 4;
    switch (WiFi.encryptionType(network)) {
      case ENC_TYPE_WEP:
        jsonNetwork["encryption"] = "wep";
        break;
      case ENC_TYPE_TKIP:
        jsonNetwork["encryption"] = "wpa";
        break;
      case ENC_TYPE_CCMP:
        jsonNetwork["encryption"] = "wpa2";
        break;
      case ENC_TYPE_NONE:
        jsonNetwork["encryption"] = "none";
        break;
      case ENC_TYPE_AUTO:
        jsonNetwork["encryption"] = "auto";
        break;
    }

    networks.add(jsonNetwork);
  }

  jsonLength++; // \0

  delete[] this->_jsonWifiNetworks;
  this->_jsonWifiNetworks = new char[jsonLength];
  json.printTo(this->_jsonWifiNetworks, jsonLength);
}

void BootConfig::_onDeviceInfoRequest() {
  this->_interface->logger->logln(F("Received device info request"));

  DynamicJsonBuffer jsonBuffer = DynamicJsonBuffer(JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(this->_interface->registeredNodesCount) + (this->_interface->registeredNodesCount * JSON_OBJECT_SIZE(2)));
  int jsonLength = 82; // {"device_id":"","homie_version":"","firmware":{"name":"","version":""},"nodes":[]}
  JsonObject& json = jsonBuffer.createObject();
  jsonLength += strlen(Helpers::getDeviceId());
  json["device_id"] = Helpers::getDeviceId();
  jsonLength += strlen(VERSION);
  json["homie_version"] = VERSION;
  JsonObject& firmware = json.createNestedObject("firmware");
  jsonLength += strlen(this->_interface->firmware.name);
  firmware["name"] = this->_interface->firmware.name;
  jsonLength += strlen(this->_interface->firmware.version);
  firmware["version"] = this->_interface->firmware.version;

  JsonArray& nodes = json.createNestedArray("nodes");
  for (int i = 0; i < this->_interface->registeredNodesCount; i++) {
    jsonLength += 20; // {"id":"","type":""},
    const HomieNode* node = this->_interface->registeredNodes[i];
    JsonObject& jsonNode = jsonBuffer.createObject();
    jsonLength += strlen(node->getId());
    jsonNode["id"] = node->getId();
    jsonLength += strlen(node->getType());
    jsonNode["type"] = node->getType();

    nodes.add(jsonNode);
  }

  jsonLength++; // \0

  std::unique_ptr<char[]> jsonString(new char[jsonLength]);
  json.printTo(jsonString.get(), jsonLength);
  this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), jsonString.get());
}

void BootConfig::_onNetworksRequest() {
  this->_interface->logger->logln(F("Received networks request"));
  if (this->_wifiScanAvailable) {
    this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), this->_jsonWifiNetworks);
  } else {
    this->_http.send(503, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_NETWORKS_FAILURE));
  }
}

void BootConfig::_onConfigRequest() {
  this->_interface->logger->logln(F("Received config request"));
  if (this->_flaggedForReboot) {
    this->_interface->logger->logln(F("✖ Device already configured"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Device already configured\"}"));
    this->_http.send(403, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> parseJsonBuffer;
  char* bodyCharArray = strdup(this->_http.arg("plain").c_str());
  JsonObject& parsedJson = parseJsonBuffer.parseObject(bodyCharArray); // do not use plain String, else fails
  if (!parsedJson.success()) {
    this->_interface->logger->logln(F("✖ Invalid or too big JSON"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Invalid or too big JSON\"}"));
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  ConfigValidationResult configValidationResult = Helpers::validateConfig(parsedJson);
  free(bodyCharArray);
  if (!configValidationResult.valid) {
    this->_interface->logger->log(F("✖ Config file is not valid, reason: "));
    this->_interface->logger->logln(configValidationResult.reason);
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Config file is not valid, reason: "));
    errorJson.concat(configValidationResult.reason);
    errorJson.concat(F("\"}"));
    this->_http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  this->_interface->config->write(this->_http.arg("plain"));

  this->_interface->logger->logln(F("✔ Configured"));

  this->_http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), F("{\"success\":true}"));

  this->_flaggedForReboot = true; // We don't reboot immediately, otherwise the response above is not sent
  this->_flaggedForRebootAt = millis();
}

void BootConfig::loop() {
  Boot::loop();

  this->_dns.processNextRequest();
  this->_http.handleClient();

  if (this->_flaggedForReboot) {
    if (millis() - this->_flaggedForRebootAt >= 3000UL) {
      this->_interface->logger->logln(F("↻ Rebooting into normal mode..."));
      ESP.restart();
    }

    return;
  }

  if (!this->_lastWifiScanEnded) {
    int8_t scanResult = WiFi.scanComplete();

    switch (scanResult) {
      case WIFI_SCAN_RUNNING:
        return;
      case WIFI_SCAN_FAILED:
        this->_interface->logger->logln(F("✖ Wi-Fi scan failed"));
        this->_ssidCount = 0;
        this->_wifiScanTimer.reset();
        break;
      default:
        this->_interface->logger->logln(F("✔ Wi-Fi scan completed"));
        this->_ssidCount = scanResult;
        this->_generateNetworksJson();
        this->_wifiScanAvailable = true;
        break;
    }

    this->_lastWifiScanEnded = true;
  }

  if (this->_lastWifiScanEnded && this->_wifiScanTimer.check()) {
    this->_interface->logger->logln(F("Triggering Wi-Fi scan..."));
    WiFi.scanNetworks(true);
    this->_wifiScanTimer.tick();
    this->_lastWifiScanEnded = false;
  }
}

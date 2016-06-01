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
, _proxyEnabled(false)
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

  WiFi.mode(WIFI_AP_STA);

  char apName[MAX_WIFI_SSID_LENGTH];
  strcpy(apName, this->_interface->brand);
  strcat_P(apName, PSTR("-"));
  strcat(apName, Helpers::getDeviceId());

  WiFi.softAPConfig(ACCESS_POINT_IP, ACCESS_POINT_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, deviceId);

  this->_interface->logger->log(F("AP started as "));
  this->_interface->logger->logln(apName);
  this->_dns.setTTL(30);
  this->_dns.setErrorReplyCode(DNSReplyCode::NoError);
  this->_dns.start(53, F("*"), ACCESS_POINT_IP);

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
  this->_http.on("/wifi-connect", HTTP_POST, std::bind(&BootConfig::_onWifiConnectRequest, this));
  this->_http.on("/wifi-status", HTTP_GET, std::bind(&BootConfig::_onWifiStatusRequest, this));
  this->_http.on("/proxy-control", HTTP_POST, std::bind(&BootConfig::_onProxyControlRequest, this));
  this->_http.onNotFound(std::bind(&BootConfig::_onCaptivePortal, this));
  this->_http.begin();
}

void BootConfig::_onWifiConnectRequest() {
  this->_interface->logger->logln(F("Received wifi connect request"));
  String ssid = this->_http.arg("ssid");
  String pass = this->_http.arg("password");
  if(ssid && pass && ssid!="" && pass!= "") {
    this->_interface->logger->logln(F("Connecting to WiFi"));
    WiFi.begin(ssid.c_str(), pass.c_str());
    this->_http.send(202, "application/json", "{\"success\":true}");
  } else {
    this->_interface->logger->logln(F("ssid/password required"));
    this->_http.send(400, "application/json", "{\"success\":false, \"error\":\"ssid-password-required\"}");
  }
}

void BootConfig::_onWifiStatusRequest() {
  this->_interface->logger->logln(F("Received wifi status request"));
  String json = "";
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      json = "{\"status\":\"idle\"}"; break;
    case WL_CONNECT_FAILED:
      json = "{\"status\":\"connect-failed\"}"; break;
    case WL_CONNECTION_LOST:
      json = "{\"status\":\"connection-lost\"}"; break;
    case WL_NO_SSID_AVAIL:
      json = "{\"status\":\"no-ssid-avail\"}"; break;
    case WL_CONNECTED:
      json = "{\"status\":\"connected\", \"ip\":\""+ WiFi.localIP().toString() +"\"}"; break;
    case WL_DISCONNECTED:
      json = "{\"status\":\"disconnected\"}"; break;
    default:
      json = "{\"status\":\"other\"}"; break;
  }
  this->_interface->logger->log(F("WiFi status "));
  this->_interface->logger->logln(json);
  this->_http.send(200, "application/json", json);
}

void BootConfig::_onProxyControlRequest() {
  this->_interface->logger->logln(F("Received proxy control request"));
  String enable = this->_http.arg("enable");
  this->_proxyEnabled = (enable == "true");
  if(this->_proxyEnabled) {
    this->_http.send(200, "application/json", "{\"message\":\"proxy-enabled\"}");
  } else {
    this->_http.send(200, "application/json", "{\"message\":\"proxy-disabled\"}");
  }
  this->_interface->logger->log(F("Transparent proxy enabled="));
  this->_interface->logger->logln(this->_proxyEnabled);
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

void BootConfig::_onCaptivePortal() {
  String host = this->_http.hostHeader();
  if (host && !host.equalsIgnoreCase(F("homie.config"))) {
    //redirect unknown host requests to self if not connected to Internet yet
    if(!this->_proxyEnabled) {
      this->_interface->logger->logln(F("Received captive portal request"));
      // Catch any captive portal probe.
      // Every browser brand uses a different URL for this purpose
      // We MUST redirect all them to local webserver to prevent cache poisoning
      this->_http.sendHeader(F("Location"), F("http://homie.config/"));
      this->_http.send(302, F("text/plain"), F(""));
    //perform transparent proxy to Internet if connected
    } else {
      this->_proxyHttpRequest();
    }
  } else if (this->_http.uri() != "/" || !SPIFFS.exists(CONFIG_UI_BUNDLE_PATH)) {
    this->_interface->logger->logln(F("Received not found request"));
    this->_http.send(404, F("text/plain"), F("UI bundle not loaded. See Configuration API usage: http://marvinroger.viewdocs.io/homie-esp8266/6.-Configuration-API"));
  } else {
    this->_interface->logger->logln(F("Received UI request"));
    File file = SPIFFS.open(CONFIG_UI_BUNDLE_PATH, "r");
    size_t sent = this->_http.streamFile(file, F("text/html"));
    file.close();
  }
}

void BootConfig::_proxyHttpRequest() {
  this->_interface->logger->logln(F("Received transparent proxy request"));

  //send request to destination (as in incoming host header)
  this->_httpClient.setUserAgent("ESP8266-Homie");
  this->_httpClient.begin("http://" + this->_http.hostHeader() + this->_http.uri());
  //copy headers
  for(int i=0; i<this->_http.headers(); i++) {
    this->_httpClient.addHeader(this->_http.headerName(i), this->_http.header(i));
  }

  String method = "";
  switch(this->_http.method()) {
    case HTTP_GET: method = "GET"; break;
    case HTTP_PUT: method = "PUT"; break;
    case HTTP_POST: method = "POST"; break;
    case HTTP_DELETE: method = "DELETE"; break;
    case HTTP_OPTIONS: method = "OPTIONS"; break;
  }

  this->_interface->logger->logln(F("Proxy sent request to destination"));
  int _httpCode = this->_httpClient.sendRequest(method.c_str(), this->_http.arg("plain"));
  this->_interface->logger->log(F("Destination response code="));
  this->_interface->logger->logln(_httpCode);

  //bridge response to browser
  //copy response headers
  for(int i=0; i<this->_httpClient.headers(); i++) {
    this->_http.sendHeader(this->_httpClient.headerName(i), this->_httpClient.header(i), false);
  }
  this->_interface->logger->logln(F("Bridging received destination contents to client"));
  this->_http.send(_httpCode, this->_httpClient.header("Content-Type"), this->_httpClient.getString());
  this->_httpClient.end();
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

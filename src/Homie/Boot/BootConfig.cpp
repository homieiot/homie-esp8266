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
, _proxyEnabled(false) {
  _wifiScanTimer.setInterval(CONFIG_SCAN_INTERVAL);
}

BootConfig::~BootConfig() {
}

void BootConfig::setup() {
  Boot::setup();

  if (_interface->led.enabled) {
    digitalWrite(_interface->led.pin, _interface->led.on);
  }

  const char* deviceId = Helpers::getDeviceId();

  _interface->logger->log(F("Device ID is "));
  _interface->logger->logln(deviceId);

  WiFi.mode(WIFI_AP_STA);

  char apName[MAX_WIFI_SSID_LENGTH];
  strcpy(apName, _interface->brand);
  strcat_P(apName, PSTR("-"));
  strcat(apName, Helpers::getDeviceId());

  WiFi.softAPConfig(ACCESS_POINT_IP, ACCESS_POINT_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, deviceId);

  _interface->logger->log(F("AP started as "));
  _interface->logger->logln(apName);
  _dns.setTTL(30);
  _dns.setErrorReplyCode(DNSReplyCode::NoError);
  _dns.start(53, F("*"), ACCESS_POINT_IP);

  _http.on("/heart", HTTP_GET, [this]() {
    _interface->logger->logln(F("Received heart request"));
    _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), F("{\"heart\":\"beat\"}"));
  });
  _http.on("/device-info", HTTP_GET, std::bind(&BootConfig::_onDeviceInfoRequest, this));
  _http.on("/networks", HTTP_GET, std::bind(&BootConfig::_onNetworksRequest, this));
  _http.on("/config", HTTP_PUT, std::bind(&BootConfig::_onConfigRequest, this));
  _http.on("/config", HTTP_OPTIONS, [this]() {  // CORS
    _interface->logger->logln(F("Received CORS request for /config"));
    _http.sendContent(FPSTR(PROGMEM_CONFIG_CORS));
  });
  _http.on("/wifi-connect", HTTP_POST, std::bind(&BootConfig::_onWifiConnectRequest, this));
  _http.on("/wifi-status", HTTP_GET, std::bind(&BootConfig::_onWifiStatusRequest, this));
  _http.on("/proxy-control", HTTP_POST, std::bind(&BootConfig::_onProxyControlRequest, this));
  _http.onNotFound(std::bind(&BootConfig::_onCaptivePortal, this));
  _http.begin();
}

void BootConfig::_onWifiConnectRequest() {
  _interface->logger->logln(F("Received wifi connect request"));
  String ssid = _http.arg("ssid");
  String pass = _http.arg("password");
  if (ssid && pass && ssid != "" && pass != "") {
    _interface->logger->logln(F("Connecting to WiFi"));
    WiFi.begin(ssid.c_str(), pass.c_str());
    _http.send(202, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"success\":true}");
  } else {
    _interface->logger->logln(F("ssid/password required"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"success\":false, \"error\":\"ssid-password-required\"}");
  }
}

void BootConfig::_onWifiStatusRequest() {
  _interface->logger->logln(F("Received wifi status request"));
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
  _interface->logger->log(F("WiFi status "));
  _interface->logger->logln(json);
  _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), json);
}

void BootConfig::_onProxyControlRequest() {
  _interface->logger->logln(F("Received proxy control request"));
  String enable = _http.arg("enable");
  _proxyEnabled = (enable == "true");
  if (_proxyEnabled) {
    _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"message\":\"proxy-enabled\"}");
  } else {
    _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), "{\"message\":\"proxy-disabled\"}");
  }
  _interface->logger->log(F("Transparent proxy enabled="));
  _interface->logger->logln(_proxyEnabled);
}

void BootConfig::_generateNetworksJson() {
  DynamicJsonBuffer generatedJsonBuffer = DynamicJsonBuffer(JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(_ssidCount) + (_ssidCount * JSON_OBJECT_SIZE(3)));  // 1 at root, 3 in childrend
  JsonObject& json = generatedJsonBuffer.createObject();

  JsonArray& networks = json.createNestedArray("networks");
  for (int network = 0; network < _ssidCount; network++) {
    JsonObject& jsonNetwork = generatedJsonBuffer.createObject();
    jsonNetwork["ssid"] = WiFi.SSID(network);
    jsonNetwork["rssi"] = WiFi.RSSI(network);
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

  delete[] _jsonWifiNetworks;
  size_t jsonBufferLength = json.measureLength() + 1;
  _jsonWifiNetworks = new char[jsonBufferLength];
  json.printTo(_jsonWifiNetworks, jsonBufferLength);
}

void BootConfig::_onCaptivePortal() {
  String host = _http.hostHeader();
  if (host && !host.equalsIgnoreCase(F("homie.config"))) {
    // redirect unknown host requests to self if not connected to Internet yet
    if (!_proxyEnabled) {
      _interface->logger->logln(F("Received captive portal request"));
      // Catch any captive portal probe.
      // Every browser brand uses a different URL for this purpose
      // We MUST redirect all them to local webserver to prevent cache poisoning
      _http.sendHeader(F("Location"), F("http://homie.config/"));
      _http.send(302, F("text/plain"), F(""));
    // perform transparent proxy to Internet if connected
    } else {
      _proxyHttpRequest();
    }
  } else if (_http.uri() != "/" || !SPIFFS.exists(CONFIG_UI_BUNDLE_PATH)) {
    _interface->logger->logln(F("Received not found request"));
    _http.send(404, F("text/plain"), F("UI bundle not loaded. See Configuration API usage: http://marvinroger.viewdocs.io/homie-esp8266/6.-Configuration-API"));
  } else {
    _interface->logger->logln(F("Received UI request"));
    File file = SPIFFS.open(CONFIG_UI_BUNDLE_PATH, "r");
    _http.streamFile(file, F("text/html"));
    file.close();
  }
}

void BootConfig::_proxyHttpRequest() {
  _interface->logger->logln(F("Received transparent proxy request"));

  // send request to destination (as in incoming host header)
  _httpClient.setUserAgent("ESP8266-Homie");
  _httpClient.begin("http://" + _http.hostHeader() + _http.uri());
  // copy headers
  for (int i = 0; i < _http.headers(); i++) {
    _httpClient.addHeader(_http.headerName(i), _http.header(i));
  }

  String method = "";
  switch (_http.method()) {
    case HTTP_GET: method = "GET"; break;
    case HTTP_PUT: method = "PUT"; break;
    case HTTP_POST: method = "POST"; break;
    case HTTP_DELETE: method = "DELETE"; break;
    case HTTP_OPTIONS: method = "OPTIONS"; break;
    default: break;
  }

  _interface->logger->logln(F("Proxy sent request to destination"));
  int _httpCode = _httpClient.sendRequest(method.c_str(), _http.arg("plain"));
  _interface->logger->log(F("Destination response code="));
  _interface->logger->logln(_httpCode);

  // bridge response to browser
  // copy response headers
  for (int i = 0; i < _httpClient.headers(); i++) {
    _http.sendHeader(_httpClient.headerName(i), _httpClient.header(i), false);
  }
  _interface->logger->logln(F("Bridging received destination contents to client"));
  _http.send(_httpCode, _httpClient.header("Content-Type"), _httpClient.getString());
  _httpClient.end();
}

void BootConfig::_onDeviceInfoRequest() {
  _interface->logger->logln(F("Received device info request"));
  auto numSettings = IHomieSetting::settings.size();
  auto numNodes = HomieNode::nodes.size();
  DynamicJsonBuffer jsonBuffer = DynamicJsonBuffer(JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(numNodes) + (numNodes * JSON_OBJECT_SIZE(2)) + JSON_ARRAY_SIZE(numSettings) + (numSettings * JSON_OBJECT_SIZE(5)));
  JsonObject& json = jsonBuffer.createObject();
  json["device_id"] = Helpers::getDeviceId();
  json["homie_esp8266_version"] = HOMIE_ESP8266_VERSION;
  JsonObject& firmware = json.createNestedObject("firmware");
  firmware["name"] = _interface->firmware.name;
  firmware["version"] = _interface->firmware.version;

  JsonArray& nodes = json.createNestedArray("nodes");
  for (HomieNode* iNode : HomieNode::nodes) {
    JsonObject& jsonNode = jsonBuffer.createObject();
    jsonNode["id"] = iNode->getId();
    jsonNode["type"] = iNode->getType();
    nodes.add(jsonNode);
  }

  JsonArray& settings = json.createNestedArray("settings");
  for (IHomieSetting* iSetting : IHomieSetting::settings) {
    JsonObject& jsonSetting = jsonBuffer.createObject();
    if (iSetting->isBool()) {
      HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);
      jsonSetting["name"] = setting->getName();
      jsonSetting["description"] = setting->getDescription();
      jsonSetting["type"] = "bool";
      jsonSetting["required"] = setting->isRequired();
      if (!setting->isRequired()) {
        jsonSetting["default"] = setting->get();
      }
    } else if (iSetting->isUnsignedLong()) {
      HomieSetting<unsigned long>* setting = static_cast<HomieSetting<unsigned long>*>(iSetting);
      jsonSetting["name"] = setting->getName();
      jsonSetting["description"] = setting->getDescription();
      jsonSetting["type"] = "ulong";
      jsonSetting["required"] = setting->isRequired();
      if (!setting->isRequired()) {
        jsonSetting["default"] = setting->get();
      }
    } else if (iSetting->isLong()) {
      HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);
      jsonSetting["name"] = setting->getName();
      jsonSetting["description"] = setting->getDescription();
      jsonSetting["type"] = "long";
      jsonSetting["required"] = setting->isRequired();
      if (!setting->isRequired()) {
        jsonSetting["default"] = setting->get();
      }
    } else if (iSetting->isDouble()) {
      HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);
      jsonSetting["name"] = setting->getName();
      jsonSetting["description"] = setting->getDescription();
      jsonSetting["type"] = "double";
      jsonSetting["required"] = setting->isRequired();
      if (!setting->isRequired()) {
        jsonSetting["default"] = setting->get();
      }
    } else if (iSetting->isConstChar()) {
      HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);
      jsonSetting["name"] = setting->getName();
      jsonSetting["description"] = setting->getDescription();
      jsonSetting["type"] = "string";
      jsonSetting["required"] = setting->isRequired();
      if (!setting->isRequired()) {
        jsonSetting["default"] = setting->get();
      }
    }

    settings.add(jsonSetting);
  }

  size_t jsonBufferLength = json.measureLength() + 1;
  std::unique_ptr<char[]> jsonString(new char[jsonBufferLength]);
  json.printTo(jsonString.get(), jsonBufferLength);
  _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), jsonString.get());
}

void BootConfig::_onNetworksRequest() {
  _interface->logger->logln(F("Received networks request"));
  if (_wifiScanAvailable) {
    _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), _jsonWifiNetworks);
  } else {
    _http.send(503, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_NETWORKS_FAILURE));
  }
}

void BootConfig::_onConfigRequest() {
  _interface->logger->logln(F("Received config request"));
  if (_flaggedForReboot) {
    _interface->logger->logln(F("✖ Device already configured"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Device already configured\"}"));
    _http.send(403, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> parseJsonBuffer;
  char* bodyCharArray = strdup(_http.arg("plain").c_str());
  JsonObject& parsedJson = parseJsonBuffer.parseObject(bodyCharArray);  // do not use plain String, else fails
  if (!parsedJson.success()) {
    _interface->logger->logln(F("✖ Invalid or too big JSON"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Invalid or too big JSON\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  ConfigValidationResult configValidationResult = Helpers::validateConfig(parsedJson);
  if (!configValidationResult.valid) {
    _interface->logger->log(F("✖ Config file is not valid, reason: "));
    _interface->logger->logln(configValidationResult.reason);
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Config file is not valid, reason: "));
    errorJson.concat(configValidationResult.reason);
    errorJson.concat(F("\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  _interface->config->write(bodyCharArray);
  free(bodyCharArray);

  _interface->logger->logln(F("✔ Configured"));

  _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), F("{\"success\":true}"));

  _flaggedForReboot = true;  // We don't reboot immediately, otherwise the response above is not sent
  _flaggedForRebootAt = millis();
}

void BootConfig::loop() {
  Boot::loop();

  _dns.processNextRequest();
  _http.handleClient();

  if (_flaggedForReboot) {
    if (millis() - _flaggedForRebootAt >= 3000UL) {
      _interface->logger->logln(F("↻ Rebooting into normal mode..."));
      _interface->logger->flush();
      ESP.restart();
    }

    return;
  }

  if (!_lastWifiScanEnded) {
    int8_t scanResult = WiFi.scanComplete();

    switch (scanResult) {
      case WIFI_SCAN_RUNNING:
        return;
      case WIFI_SCAN_FAILED:
        _interface->logger->logln(F("✖ Wi-Fi scan failed"));
        _ssidCount = 0;
        _wifiScanTimer.reset();
        break;
      default:
        _interface->logger->logln(F("✔ Wi-Fi scan completed"));
        _ssidCount = scanResult;
        _generateNetworksJson();
        _wifiScanAvailable = true;
        break;
    }

    _lastWifiScanEnded = true;
  }

  if (_lastWifiScanEnded && _wifiScanTimer.check()) {
    _interface->logger->logln(F("Triggering Wi-Fi scan..."));
    WiFi.scanNetworks(true);
    _wifiScanTimer.tick();
    _lastWifiScanEnded = false;
  }
}

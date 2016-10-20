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

  if (Interface::get().led.enabled) {
    digitalWrite(Interface::get().led.pin, Interface::get().led.on);
  }

  const char* deviceId = DeviceId::get();

  Interface::get().logger->print(F("Device ID is "));
  Interface::get().logger->println(deviceId);

  WiFi.mode(WIFI_AP_STA);

  char apName[MAX_WIFI_SSID_LENGTH];
  strcpy(apName, Interface::get().brand);
  strcat_P(apName, PSTR("-"));
  strcat(apName, DeviceId::get());

  WiFi.softAPConfig(ACCESS_POINT_IP, ACCESS_POINT_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, deviceId);

  Interface::get().logger->print(F("AP started as "));
  Interface::get().logger->println(apName);
  _dns.setTTL(30);
  _dns.setErrorReplyCode(DNSReplyCode::NoError);
  _dns.start(53, F("*"), ACCESS_POINT_IP);

  _http.on("/heart", HTTP_GET, [this]() {
    Interface::get().logger->println(F("Received heart request"));
    _http.send(204);
  });
  _http.on("/device-info", HTTP_GET, std::bind(&BootConfig::_onDeviceInfoRequest, this));
  _http.on("/networks", HTTP_GET, std::bind(&BootConfig::_onNetworksRequest, this));
  _http.on("/config", HTTP_PUT, std::bind(&BootConfig::_onConfigRequest, this));
  _http.on("/config", HTTP_OPTIONS, [this]() {  // CORS
    Interface::get().logger->println(F("Received CORS request for /config"));
    _http.sendContent(FPSTR(PROGMEM_CONFIG_CORS));
  });
  _http.on("/wifi/connect", HTTP_PUT, std::bind(&BootConfig::_onWifiConnectRequest, this));
  _http.on("/wifi/status", HTTP_GET, std::bind(&BootConfig::_onWifiStatusRequest, this));
  _http.on("/proxy/control", HTTP_PUT, std::bind(&BootConfig::_onProxyControlRequest, this));
  _http.onNotFound(std::bind(&BootConfig::_onCaptivePortal, this));
  _http.begin();
}

void BootConfig::_onWifiConnectRequest() {
  Interface::get().logger->println(F("Received Wi-Fi connect request"));
  StaticJsonBuffer<JSON_OBJECT_SIZE(2)> parseJsonBuffer;
  std::unique_ptr<char[]> bodyString = Helpers::cloneString(_http.arg("plain"));
  JsonObject& parsedJson = parseJsonBuffer.parseObject(bodyString.get());
  if (!parsedJson.success()) {
    Interface::get().logger->println(F("✖ Invalid or too big JSON"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Invalid or too big JSON\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  if (!parsedJson.containsKey("ssid") || !parsedJson["ssid"].is<const char*>() || !parsedJson.containsKey("password") || !parsedJson["password"].is<const char*>()) {
    Interface::get().logger->println(F("✖ SSID and password required"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("SSID and password required\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  Interface::get().logger->println(F("Connecting to Wi-Fi"));
  WiFi.begin(parsedJson["ssid"].as<const char*>(), parsedJson["password"].as<const char*>());
  _http.send(202, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), F("{\"success\":true}"));
}

void BootConfig::_onWifiStatusRequest() {
  Interface::get().logger->println(F("Received Wi-Fi status request"));
  String json = "";
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      json = F("{\"status\":\"idle\"}");
      break;
    case WL_CONNECT_FAILED:
      json = F("{\"status\":\"connect_failed\"}");
      break;
    case WL_CONNECTION_LOST:
      json = F("{\"status\":\"connection_lost\"}");
      break;
    case WL_NO_SSID_AVAIL:
      json = F("{\"status\":\"no_ssid_available\"}");
      break;
    case WL_CONNECTED:
      json = "{\"status\":\"connected\",\"local_ip\":\"" + WiFi.localIP().toString() + "\"}";
      break;
    case WL_DISCONNECTED:
      json = F("{\"status\":\"disconnected\"}");
      break;
    default:
      json = F("{\"status\":\"other\"}");
      break;
  }

  _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), json);
}

void BootConfig::_onProxyControlRequest() {
  Interface::get().logger->println(F("Received proxy control request"));
  StaticJsonBuffer<JSON_OBJECT_SIZE(1)> parseJsonBuffer;
  std::unique_ptr<char[]> bodyString = Helpers::cloneString(_http.arg("plain"));
  JsonObject& parsedJson = parseJsonBuffer.parseObject(bodyString.get());  // do not use plain String, else fails
  if (!parsedJson.success()) {
    Interface::get().logger->println(F("✖ Invalid or too big JSON"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Invalid or too big JSON\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  if (!parsedJson.containsKey("enable") || !parsedJson["enable"].is<bool>()) {
    Interface::get().logger->println(F("✖ enable parameter is required"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("enable parameter is required\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  _proxyEnabled = parsedJson["enable"];
  _http.send(202, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), F("{\"success\":true}"));
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
      Interface::get().logger->println(F("Received captive portal request"));
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
    Interface::get().logger->println(F("Received not found request"));
    _http.send(404, F("text/plain"), F("UI bundle not loaded. See Configuration API usage: https://homie-esp8266.readme.io/docs/http-json-api"));
  } else {
    Interface::get().logger->println(F("Received UI request"));
    File file = SPIFFS.open(CONFIG_UI_BUNDLE_PATH, "r");
    _http.streamFile(file, F("text/html"));
    file.close();
  }
}

void BootConfig::_proxyHttpRequest() {
  Interface::get().logger->println(F("Received transparent proxy request"));

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

  Interface::get().logger->println(F("Proxy sent request to destination"));
  int _httpCode = _httpClient.sendRequest(method.c_str(), _http.arg("plain"));
  Interface::get().logger->print(F("Destination response code="));
  Interface::get().logger->println(_httpCode);

  // bridge response to browser
  // copy response headers
  for (int i = 0; i < _httpClient.headers(); i++) {
    _http.sendHeader(_httpClient.headerName(i), _httpClient.header(i), false);
  }
  Interface::get().logger->println(F("Bridging received destination contents to client"));
  _http.send(_httpCode, _httpClient.header("Content-Type"), _httpClient.getString());
  _httpClient.end();
}

void BootConfig::_onDeviceInfoRequest() {
  Interface::get().logger->println(F("Received device information request"));
  auto numSettings = IHomieSetting::settings.size();
  auto numNodes = HomieNode::nodes.size();
  DynamicJsonBuffer jsonBuffer = DynamicJsonBuffer(JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(numNodes) + (numNodes * JSON_OBJECT_SIZE(2)) + JSON_ARRAY_SIZE(numSettings) + (numSettings * JSON_OBJECT_SIZE(5)));
  JsonObject& json = jsonBuffer.createObject();
  json["hardware_device_id"] = DeviceId::get();
  json["homie_esp8266_version"] = HOMIE_ESP8266_VERSION;
  JsonObject& firmware = json.createNestedObject("firmware");
  firmware["name"] = Interface::get().firmware.name;
  firmware["version"] = Interface::get().firmware.version;

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
  Interface::get().logger->println(F("Received networks request"));
  if (_wifiScanAvailable) {
    _http.send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), _jsonWifiNetworks);
  } else {
    _http.send(503, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_NETWORKS_FAILURE));
  }
}

void BootConfig::_onConfigRequest() {
  Interface::get().logger->println(F("Received config request"));
  if (_flaggedForReboot) {
    Interface::get().logger->println(F("✖ Device already configured"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Device already configured\"}"));
    _http.send(403, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  StaticJsonBuffer<MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE> parseJsonBuffer;
  std::unique_ptr<char[]> bodyString = Helpers::cloneString(_http.arg("plain"));
  JsonObject& parsedJson = parseJsonBuffer.parseObject(bodyString.get());  // workaround, cannot pass raw String otherwise JSON parsing fails randomly
  if (!parsedJson.success()) {
    Interface::get().logger->println(F("✖ Invalid or too big JSON"));
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Invalid or too big JSON\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  ConfigValidationResult configValidationResult = Validation::validateConfig(parsedJson);
  if (!configValidationResult.valid) {
    Interface::get().logger->print(F("✖ Config file is not valid, reason: "));
    Interface::get().logger->println(configValidationResult.reason);
    String errorJson = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
    errorJson.concat(F("Config file is not valid, reason: "));
    errorJson.concat(configValidationResult.reason);
    errorJson.concat(F("\"}"));
    _http.send(400, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
    return;
  }

  Interface::get().config->write(parsedJson);

  Interface::get().logger->println(F("✔ Configured"));

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
      Interface::get().logger->println(F("↻ Rebooting into normal mode..."));
      Serial.flush();
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
        Interface::get().logger->println(F("✖ Wi-Fi scan failed"));
        _ssidCount = 0;
        _wifiScanTimer.reset();
        break;
      default:
        Interface::get().logger->println(F("✔ Wi-Fi scan completed"));
        _ssidCount = scanResult;
        _generateNetworksJson();
        _wifiScanAvailable = true;
        break;
    }

    _lastWifiScanEnded = true;
  }

  if (_lastWifiScanEnded && _wifiScanTimer.check()) {
    Interface::get().logger->println(F("Triggering Wi-Fi scan..."));
    WiFi.scanNetworks(true);
    _wifiScanTimer.tick();
    _lastWifiScanEnded = false;
  }
}

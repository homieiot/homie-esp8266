#include "BootConfig.hpp"

#if HOMIE_CONFIG
using namespace HomieInternals;

BootConfig::BootConfig()
  : Boot("config")
  , _http(80)
  , _httpClient()
  , _ssidCount(0)
  , _wifiScanAvailable(false)
  , _lastWifiScanEnded(true)
  , _jsonWifiNetworks()
  , _flaggedForReboot(false)
  , _flaggedForRebootAt(0)
  , _proxyEnabled(false)
  , _apIpStr{ '\0' }
{
  _wifiScanTimer.setInterval(CONFIG_SCAN_INTERVAL);
}

BootConfig::~BootConfig() {
}

void BootConfig::setup() {
  Boot::setup();

  if (Interface::get().led.enabled) {
    digitalWrite(Interface::get().led.pin, Interface::get().led.on);
  }

  Interface::get().getLogger() << F("Device ID is ") << DeviceId::get() << endl;

  WiFi.mode(WIFI_AP_STA);

  char apName[MAX_WIFI_SSID_LENGTH];
  strlcpy(apName, Interface::get().brand, MAX_WIFI_SSID_LENGTH - 1 - MAX_MAC_STRING_LENGTH);
  strcat_P(apName, PSTR("-"));
  strcat(apName, DeviceId::get());

  WiFi.softAPConfig(ACCESS_POINT_IP, ACCESS_POINT_IP, IPAddress(255, 255, 255, 0));
  if (Interface::get().configurationAp.secured) {
    WiFi.softAP(apName, Interface::get().configurationAp.password);
  } else {
    WiFi.softAP(apName);
  }

  Helpers::ipToString(ACCESS_POINT_IP, _apIpStr);

  Interface::get().getLogger() << F("AP started as ") << apName << F(" with IP ") << _apIpStr << endl;
  _dns.setTTL(30);
  _dns.setErrorReplyCode(DNSReplyCode::NoError);
  _dns.start(53, F("*"), ACCESS_POINT_IP);

  __setCORS();
  _http.on("/heart", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Interface::get().getLogger() << F("Received heart request") << endl;
    request->send(204);
  });
  _http.on("/device-info", HTTP_GET, [this](AsyncWebServerRequest *request) { _onDeviceInfoRequest(request); });
  _http.on("/networks", HTTP_GET, [this](AsyncWebServerRequest *request) { _onNetworksRequest(request); });
  _http.on("/config", HTTP_PUT, [this](AsyncWebServerRequest *request) { _onConfigRequest(request); }).onBody(BootConfig::__parsePost);
  _http.on("/wifi/connect", HTTP_PUT, [this](AsyncWebServerRequest *request) { _onWifiConnectRequest(request); }).onBody(BootConfig::__parsePost);
  _http.on("/wifi/status", HTTP_GET, [this](AsyncWebServerRequest *request) { _onWifiStatusRequest(request); });
  _http.on("/proxy/control", HTTP_PUT, [this](AsyncWebServerRequest *request) { _onProxyControlRequest(request); }).onBody(BootConfig::__parsePost);
  _http.onNotFound([this](AsyncWebServerRequest *request) {
    if ( request->method() == HTTP_OPTIONS ) {
      Interface::get().getLogger() << F("Received CORS request for ")<< request->url() << endl;
      request->send(200);
    } else {
      _onCaptivePortal(request);
    }
  });
  _http.begin();
}

void BootConfig::loop() {
  Boot::loop();

  _dns.processNextRequest();

  if (_flaggedForReboot) {
    if (millis() - _flaggedForRebootAt >= 3000UL) {
      Interface::get().getLogger() << F("↻ Rebooting into normal mode...") << endl;
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
      Interface::get().getLogger() << F("✖ Wi-Fi scan failed") << endl;
      _ssidCount = 0;
      _wifiScanTimer.reset();
      break;
    default:
      Interface::get().getLogger() << F("✔ Wi-Fi scan completed") << endl;
      _ssidCount = scanResult;
      _generateNetworksJson();
      _wifiScanAvailable = true;
      break;
    }

    _lastWifiScanEnded = true;
  }

  if (_lastWifiScanEnded && _wifiScanTimer.check()) {
    Interface::get().getLogger() << F("Triggering Wi-Fi scan...") << endl;
    WiFi.scanNetworks(true);
    _wifiScanTimer.tick();
    _lastWifiScanEnded = false;
  }
}

void BootConfig::_onWifiConnectRequest(AsyncWebServerRequest *request) {
  Interface::get().getLogger() << F("Received Wi-Fi connect request") << endl;
  DynamicJsonBuffer parseJsonBuffer(JSON_OBJECT_SIZE(2));
  const char* body = (const char*)(request->_tempObject);
  JsonObject& parsedJson = parseJsonBuffer.parseObject(body);
  if (!parsedJson.success()) {
    __SendJSONError(request, F("✖ Invalid or too big JSON"));
    return;
  }

  if (!parsedJson.containsKey("ssid") || !parsedJson["ssid"].is<const char*>() || !parsedJson.containsKey("password") || !parsedJson["password"].is<const char*>()) {
    __SendJSONError(request, F("✖ SSID and password required"));
    return;
  }

  Interface::get().getLogger() << F("Connecting to Wi-Fi") << endl;
  WiFi.begin(parsedJson["ssid"].as<const char*>(), parsedJson["password"].as<const char*>());

  request->send(202, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_SUCCESS));
}

void BootConfig::_onWifiStatusRequest(AsyncWebServerRequest *request) {
  Interface::get().getLogger() << F("Received Wi-Fi status request") << endl;

  DynamicJsonBuffer generatedJsonBuffer(JSON_OBJECT_SIZE(2));
  JsonObject& json = generatedJsonBuffer.createObject();
  String status;

  //String json = "";
  switch (WiFi.status()) {
  case WL_IDLE_STATUS:
    status = F("idle");
    break;
  case WL_CONNECT_FAILED:
    status = F("connect_failed");
    break;
  case WL_CONNECTION_LOST:
    status = F("connection_lost");
    break;
  case WL_NO_SSID_AVAIL:
    status = F("no_ssid_available");
    break;
  case WL_CONNECTED:
    status = F("connected");
    json["local_ip"] = WiFi.localIP().toString();
    break;
  case WL_DISCONNECTED:
    status = F("disconnected");
    break;
  default:
    status = F("other");
    break;
  }

  json["status"] = status;
  String output;
  json.printTo(output);

  request->send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), output);
}

void BootConfig::_onProxyControlRequest(AsyncWebServerRequest *request) {
  Interface::get().getLogger() << F("Received proxy control request") << endl;
  DynamicJsonBuffer parseJsonBuffer(JSON_OBJECT_SIZE(1));
  const char* body = (const char*)(request->_tempObject);
  JsonObject& parsedJson = parseJsonBuffer.parseObject(body);  // do not use plain String, else fails
  if (!parsedJson.success()) {
    __SendJSONError(request, F("✖ Invalid or too big JSON"));
    return;
  }

  if (!parsedJson.containsKey("enable") || !parsedJson["enable"].is<bool>()) {
    __SendJSONError(request, F("✖ enable parameter is required"));
    return;
  }

  _proxyEnabled = parsedJson["enable"];

  request->send(202, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_SUCCESS));
}

void BootConfig::_generateNetworksJson() {
  DynamicJsonBuffer generatedJsonBuffer(JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(_ssidCount) + (_ssidCount * JSON_OBJECT_SIZE(3)));  // 1 at root, 3 in childrend
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

  String output;
  json.printTo(output);
  _jsonWifiNetworks = output;
}

void BootConfig::_onCaptivePortal(AsyncWebServerRequest *request) {
  String host = request->host();
  Interface::get().getLogger() << F("Received captive portal request: ");
  if (host && !host.equals(_apIpStr)) {
    // redirect unknown host requests to self if not connected to Internet yet
    if (!_proxyEnabled) {
      // Catch any captive portal probe.
      // Every browser brand uses a different URL for this purpose
      // We MUST redirect all them to local webserver to prevent cache poisoning
      String redirectUrl = String("http://");
      redirectUrl.concat(_apIpStr);
      Interface::get().getLogger() << F("Redirect: ") << redirectUrl << endl;
      request->redirect(redirectUrl);
    } else {
      // perform transparent proxy to Internet if connected
      Interface::get().getLogger() << F("Proxy") << endl;
      _proxyHttpRequest(request);
    }
  } else if (request->url() == "/" && !SPIFFS.exists(CONFIG_UI_BUNDLE_PATH)) {
    // UI File not found
    String msg = String(F("UI bundle not loaded. See Configuration API usage: http://homieiot.github.io/homie-esp8266"));
    Interface::get().getLogger() << msg << endl;
    request->send(404, F("text/plain"), msg);
  } else if (request->url() == "/" && SPIFFS.exists(CONFIG_UI_BUNDLE_PATH)) {
    // Respond with UI
    Interface::get().getLogger() << F("UI bundle found") << endl;
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS.open(CONFIG_UI_BUNDLE_PATH, "r"), F("index.html"), F("text/html"));
    request->send(response);
  } else {
    // Faild to find request
    String msg = String(F("Request NOT found for url: ")) + request->url();
    Interface::get().getLogger() << msg << endl;
    request->send(404, F("text/plain"), msg);
  }
}

void BootConfig::_proxyHttpRequest(AsyncWebServerRequest *request) {
  Interface::get().getLogger() << F("Received transparent proxy request") << endl;

  String url = String("http://");
  url.concat(request->host());
  url.concat(request->url());

  // send request to destination (as in incoming host header)
  _httpClient.setUserAgent(F("ESP8266-Homie"));
  _httpClient.begin(url);
  // copy headers
  for (size_t i = 0; i < request->headers(); i++) {
    _httpClient.addHeader(request->headerName(i), request->header(i));
  }

  String method = "";
  switch (request->method()) {
  case HTTP_GET: method = F("GET"); break;
  case HTTP_PUT: method = F("PUT"); break;
  case HTTP_POST: method = F("POST"); break;
  case HTTP_DELETE: method = F("DELETE"); break;
  case HTTP_OPTIONS: method = F("OPTIONS"); break;
  default: break;
  }

  Interface::get().getLogger() << F("Proxy sent request to destination") << endl;
  const char* body = (const char*)(request->_tempObject);
  int _httpCode = _httpClient.sendRequest(method.c_str(), body);
  Interface::get().getLogger() << F("Destination response code = ") << _httpCode << endl;

  // bridge response to browser
  // copy response headers
  Interface::get().getLogger() << F("Bridging received destination contents to client") << endl;
  AsyncWebServerResponse* response = request->beginResponse(_httpCode, _httpClient.header("Content-Type"), _httpClient.getString());
  for (int i = 0; i < _httpClient.headers(); i++) {
    response->addHeader(_httpClient.headerName(i), _httpClient.header(i));
  }
  request->send(response);
  _httpClient.end();
}

void BootConfig::_onDeviceInfoRequest(AsyncWebServerRequest *request) {
  Interface::get().getLogger() << F("Received device information request") << endl;
  auto numSettings = IHomieSetting::settings.size();
  auto numNodes = HomieNode::nodes.size();
  DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(numNodes) + (numNodes * JSON_OBJECT_SIZE(2)) + JSON_ARRAY_SIZE(numSettings) + (numSettings * JSON_OBJECT_SIZE(5)));
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

    if (strcmp(iSetting->getType(), "unknown") != 0) {
      jsonSetting["name"] = iSetting->getName();
      jsonSetting["description"] = iSetting->getDescription();
      jsonSetting["type"] = iSetting->getType();
      jsonSetting["required"] = iSetting->isRequired();

      if (!iSetting->isRequired()) {
        if (iSetting->isBool()) {
          HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);
          jsonSetting["default"] = setting->get();
        } else if (iSetting->isLong()) {
          HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);
          jsonSetting["default"] = setting->get();
        } else if (iSetting->isDouble()) {
          HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);
          jsonSetting["default"] = setting->get();
        } else if (iSetting->isConstChar()) {
          HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);
          jsonSetting["default"] = setting->get();
        }
      }
    }

    settings.add(jsonSetting);
  }

  String output;
  json.printTo(output);

  request->send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), output);
}

void BootConfig::_onNetworksRequest(AsyncWebServerRequest *request) {
  Interface::get().getLogger() << F("Received networks request") << endl;
  if (_wifiScanAvailable) {
    request->send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), _jsonWifiNetworks);
  } else {
    __SendJSONError(request, F("Initial Wi-Fi scan not finished yet"), 503);
  }
}

void BootConfig::_onConfigRequest(AsyncWebServerRequest *request) {
  Interface::get().getLogger() << F("Received config request") << endl;
  if (_flaggedForReboot) {
    __SendJSONError(request, F("✖ Device already configured"), 403);
    return;
  }

  DynamicJsonBuffer parseJsonBuffer(MAX_JSON_CONFIG_ARDUINOJSON_BUFFER_SIZE);
  const char* body = (const char*)(request->_tempObject);
  JsonObject& parsedJson = parseJsonBuffer.parseObject(body);
  if (!parsedJson.success()) {
    __SendJSONError(request, F("✖ Invalid or too big JSON"));
    return;
  }

  ConfigValidationResult configValidationResult = Validation::validateConfig(parsedJson);
  if (!configValidationResult.valid) {
    __SendJSONError(request, String(F("✖ Config file is not valid, reason: ")) + configValidationResult.reason);
    return;
  }

  Interface::get().getConfig().write(parsedJson);

  Interface::get().getLogger() << F("✔ Configured") << endl;

  request->send(200, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), FPSTR(PROGMEM_CONFIG_JSON_SUCCESS));

  Interface::get().disable = true;
  _flaggedForReboot = true;  // We don't reboot immediately, otherwise the response above is not sent
  _flaggedForRebootAt = millis();
}

void BootConfig::__setCORS() {
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), F("GET, PUT"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("Content-Type, Origin, Referer, User-Agent"));
}

void BootConfig::__parsePost(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (total > MAX_POST_SIZE) {
    Interface::get().getLogger() << F("Request is to large to be processed.") << endl;
  } else {
    if (index == 0) {
      request->_tempObject = new char[total + 1];
    }
    char* buff = reinterpret_cast<char*>(request->_tempObject) + index;
    memcpy(buff, data, len);
    if (index + len == total) {
      char* buff =  reinterpret_cast<char*>(request->_tempObject) + total;
      *buff = '\0';
    }
  }
}

void HomieInternals::BootConfig::__SendJSONError(AsyncWebServerRequest * request, String msg, int16_t code) {
  Interface::get().getLogger() << msg << endl;
  const String BEGINNING = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_BEGINNING));
  const String END = String(FPSTR(PROGMEM_CONFIG_JSON_FAILURE_END));
  String errorJson = BEGINNING + msg + END;
  request->send(code, FPSTR(PROGMEM_CONFIG_APPLICATION_JSON), errorJson);
}
#endif

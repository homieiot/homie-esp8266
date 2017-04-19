#include "BootNormal.hpp"

using namespace HomieInternals;

BootNormal::BootNormal()
: Boot("normal")
, _mqttTimedRetry(MQTT_RECONNECT_STEP_INTERVAL, MQTT_RECONNECT_MAX_INTERVAL)
, _setupFunctionCalled(false)
, _mqttDisconnectNotified(true)
, _flaggedForOta(false)
, _flaggedForReset(false)
, _flaggedForReboot(false)
, _mqttOfflineMessageId(0)
, _otaIsBase64(false)
, _otaBase64Pads(0)
, _otaSizeTotal(0)
, _otaSizeDone(0)
, _mqttTopic(nullptr)
, _mqttClientId(nullptr)
, _mqttWillTopic(nullptr)
, _mqttPayloadBuffer(nullptr) {
  _statsTimer.setInterval(STATS_SEND_INTERVAL);
  strlcpy(_fwChecksum, ESP.getSketchMD5().c_str(), sizeof(_fwChecksum));
  _fwChecksum[sizeof(_fwChecksum) - 1] = '\0';
}

BootNormal::~BootNormal() {
}

void BootNormal::_prefixMqttTopic() {
  strcpy(_mqttTopic.get(), Interface::get().getConfig().get().mqtt.baseTopic);
  strcat(_mqttTopic.get(), Interface::get().getConfig().get().deviceId);
}

char* BootNormal::_prefixMqttTopic(PGM_P topic) {
  _prefixMqttTopic();
  strcat_P(_mqttTopic.get(), topic);

  return _mqttTopic.get();
}

uint16_t BootNormal::_publishOtaStatus(int status, const char* info) {
  String payload(status);
  if (info) {
    payload += ' ';
    payload += info;
  }
  return Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/ota/status")), 1, true, payload.c_str());
}

uint16_t BootNormal::_publishOtaStatus_P(int status, PGM_P info) {
  return _publishOtaStatus(status, String(info).c_str());
}

void BootNormal::_endOtaUpdate(bool success, uint8_t update_error) {
  if (success) {
    Interface::get().getLogger() << F("✔ OTA succeeded") << endl;
    Interface::get().getLogger() << F("Triggering OTA_SUCCESSFUL event...") << endl;
    Interface::get().event.type = HomieEventType::OTA_SUCCESSFUL;
    Interface::get().eventHandler(Interface::get().event);

    _publishOtaStatus(200);  // 200 OK
    _flaggedForReboot = true;
  } else {
    int code;
    String info;
    switch (update_error) {
      case UPDATE_ERROR_SIZE:               // new firmware size is zero
      case UPDATE_ERROR_MAGIC_BYTE:         // new firmware does not have 0xE9 in first byte
      case UPDATE_ERROR_NEW_FLASH_CONFIG:   // bad new flash config (does not match flash ID)
        code = 400;  // 400 Bad Request
        info = PSTR("BAD_FIRMWARE");
        break;
      case UPDATE_ERROR_MD5:
        code = 400;  // 400 Bad Request
        info = PSTR("BAD_CHECKSUM");
        break;
      case UPDATE_ERROR_SPACE:
        code = 400;  // 400 Bad Request
        info = PSTR("NOT_ENOUGH_SPACE");
        break;
      case UPDATE_ERROR_WRITE:
      case UPDATE_ERROR_ERASE:
      case UPDATE_ERROR_READ:
        code = 500;  // 500 Internal Server Error
        info = PSTR("FLASH_ERROR");
        break;
      default:
        code = 500;  // 500 Internal Server Error
        info = PSTR("INTERNAL_ERROR ") + update_error;
        break;
    }
    _publishOtaStatus(code, info.c_str());

    Interface::get().getLogger() << F("✖ OTA failed (") << code << F(" ") << info << F(")") << endl;

    Interface::get().getLogger() << F("Triggering OTA_FAILED event...") << endl;
    Interface::get().event.type = HomieEventType::OTA_FAILED;
    Interface::get().eventHandler(Interface::get().event);
  }
  _flaggedForOta = false;
}

void BootNormal::_wifiConnect() {
  if (!Interface::get().flaggedForSleep) {
    if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
    Interface::get().getLogger() << F("↕ Attempting to connect to Wi-Fi...") << endl;

    if (WiFi.getMode() != WIFI_STA) WiFi.mode(WIFI_STA);

    WiFi.hostname(Interface::get().getConfig().get().deviceId);
    if (strcmp_P(Interface::get().getConfig().get().wifi.ip, PSTR("")) != 0) {  // on _validateConfigWifi there is a requirement for mask and gateway
      byte convertedBytes[4];
      Helpers::stringToBytes(Interface::get().getConfig().get().wifi.ip, '.', convertedBytes, 4, 10);
      IPAddress convertedIp(convertedBytes[0], convertedBytes[1], convertedBytes[2], convertedBytes[3]);
      Helpers::stringToBytes(Interface::get().getConfig().get().wifi.mask, '.', convertedBytes, 4, 10);
      IPAddress convertedMask(convertedBytes[0], convertedBytes[1], convertedBytes[2], convertedBytes[3]);
      Helpers::stringToBytes(Interface::get().getConfig().get().wifi.gw, '.', convertedBytes, 4, 10);
      IPAddress convertedGateway(convertedBytes[0], convertedBytes[1], convertedBytes[2], convertedBytes[3]);

      if (strcmp_P(Interface::get().getConfig().get().wifi.dns1, PSTR("")) != 0) {
        Helpers::stringToBytes(Interface::get().getConfig().get().wifi.dns1, '.', convertedBytes, 4, 10);
        IPAddress convertedDns1(convertedBytes[0], convertedBytes[1], convertedBytes[2], convertedBytes[3]);
        if ((strcmp_P(Interface::get().getConfig().get().wifi.dns2, PSTR("")) != 0)) {  // on _validateConfigWifi there is requirement that we need dns1 if we want to define dns2
          Helpers::stringToBytes(Interface::get().getConfig().get().wifi.dns2, '.', convertedBytes, 4, 10);
          IPAddress convertedDns2(convertedBytes[0], convertedBytes[1], convertedBytes[2], convertedBytes[3]);
          WiFi.config(convertedIp, convertedGateway, convertedMask, convertedDns1, convertedDns2);
        } else {
          WiFi.config(convertedIp, convertedGateway, convertedMask, convertedDns1);
        }
      } else {
        WiFi.config(convertedIp, convertedGateway, convertedMask);
      }
    }

    if (strcmp_P(Interface::get().getConfig().get().wifi.bssid, PSTR("")) != 0) {
      byte bssidBytes[6];
      Helpers::stringToBytes(Interface::get().getConfig().get().wifi.bssid, ':', bssidBytes, 6, 16);
      WiFi.begin(Interface::get().getConfig().get().wifi.ssid, Interface::get().getConfig().get().wifi.password, Interface::get().getConfig().get().wifi.channel, bssidBytes);
    } else {
      WiFi.begin(Interface::get().getConfig().get().wifi.ssid, Interface::get().getConfig().get().wifi.password);
    }

    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  }
}

void BootNormal::_onWifiGotIp(const WiFiEventStationModeGotIP& event) {
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();
  Interface::get().getLogger() << F("✔ Wi-Fi connected, IP: ") << event.ip << endl;
  Interface::get().getLogger() << F("Triggering WIFI_CONNECTED event...") << endl;
  Interface::get().event.type = HomieEventType::WIFI_CONNECTED;
  Interface::get().event.ip = event.ip;
  Interface::get().event.mask = event.mask;
  Interface::get().event.gateway = event.gw;
  Interface::get().eventHandler(Interface::get().event);
  MDNS.begin(Interface::get().getConfig().get().deviceId);

  _mqttConnect();
}

void BootNormal::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event) {
  Interface::get().connected = false;
  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);
  _statsTimer.reset();
  Interface::get().getLogger() << F("✖ Wi-Fi disconnected") << endl;
  Interface::get().getLogger() << F("Triggering WIFI_DISCONNECTED event...") << endl;
  Interface::get().event.type = HomieEventType::WIFI_DISCONNECTED;
  Interface::get().event.wifiReason = event.reason;
  Interface::get().eventHandler(Interface::get().event);

  _wifiConnect();
}

void BootNormal::_mqttConnect() {
  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_MQTT_DELAY);
  Interface::get().getLogger() << F("↕ Attempting to connect to MQTT...") << endl;
  Interface::get().getMqttClient().connect();
}

void BootNormal::_onMqttConnected() {
  _mqttDisconnectNotified = false;
  _mqttTimedRetry.deactivate();

  Interface::get().getLogger() << F("Sending initial information...") << endl;

  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$homie")), 1, true, HOMIE_VERSION);
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation")), 1, true, "esp8266");
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$mac")), 1, true, WiFi.macAddress().c_str());

  for (HomieNode* iNode : HomieNode::nodes) {
    std::unique_ptr<char[]> subtopic = std::unique_ptr<char[]>(new char[1 + strlen(iNode->getId()) + 12 + 1]);  // /id/$properties
    strcpy_P(subtopic.get(), PSTR("/"));
    strcat(subtopic.get(), iNode->getId());
    strcat_P(subtopic.get(), PSTR("/$type"));
    Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, iNode->getType());

    strcpy_P(subtopic.get(), PSTR("/"));
    strcat(subtopic.get(), iNode->getId());
    strcat_P(subtopic.get(), PSTR("/$properties"));
    String properties;
    for (Property* iProperty : iNode->getProperties()) {
      properties.concat(iProperty->getProperty());
      if (iProperty->isRange()) {
        properties.concat("[");
        properties.concat(iProperty->getLower());
        properties.concat("-");
        properties.concat(iProperty->getUpper());
        properties.concat("]");
      }
      if (iProperty->isSettable()) properties.concat(":settable");
      properties.concat(",");
    }
    if (iNode->getProperties().size() >= 1) properties.remove(properties.length() - 1);
    Interface::get().getMqttClient().publish(_prefixMqttTopic(subtopic.get()), 1, true, properties.c_str());
  }

  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$name")), 1, true, Interface::get().getConfig().get().name);

  IPAddress localIp = WiFi.localIP();
  char localIpStr[MAX_IP_STRING_LENGTH];
  snprintf(localIpStr, MAX_IP_STRING_LENGTH, "%d.%d.%d.%d", localIp[0], localIp[1], localIp[2], localIp[3]);

  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$localip")), 1, true, localIpStr);

  char statsIntervalStr[3 + 1];
  itoa(STATS_SEND_INTERVAL / 1000, statsIntervalStr, 10);
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats/interval")), 1, true, statsIntervalStr);

  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$fw/name")), 1, true, Interface::get().firmware.name);
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$fw/version")), 1, true, Interface::get().firmware.version);
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$fw/checksum")), 1, true, _fwChecksum);

  Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/+/+/set")), 2);

  /* Implementation specific */

  char* safeConfigFile = Interface::get().getConfig().getSafeConfigFile();
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/config")), 1, true, safeConfigFile);
  free(safeConfigFile);
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/version")), 1, true, HOMIE_ESP8266_VERSION);
  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/ota/enabled")), 1, true, Interface::get().getConfig().get().ota.enabled ? "true" : "false");
  Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/$implementation/ota/firmware")), 0);
  Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/$implementation/ota/checksum")), 0);
  Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/$implementation/reset")), 2);
  Interface::get().getMqttClient().subscribe(_prefixMqttTopic(PSTR("/$implementation/config/set")), 2);

  /** Euphi: TODO #142: Homie $broadcast */
  String broadcast_topic(Interface::get().getConfig().get().mqtt.baseTopic);
  broadcast_topic.concat("$broadcast/+");
  Interface::get().getMqttClient().subscribe(broadcast_topic.c_str(), 2);

  Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$online")), 1, true, "true");

  Interface::get().connected = true;
  if (Interface::get().led.enabled) Interface::get().getBlinker().stop();

  Interface::get().getLogger() << F("✔ MQTT ready") << endl;
  Interface::get().getLogger() << F("Triggering MQTT_CONNECTED event...") << endl;
  Interface::get().event.type = HomieEventType::MQTT_CONNECTED;
  Interface::get().eventHandler(Interface::get().event);

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->onReadyToOperate();
  }

  if (!_setupFunctionCalled) {
    Interface::get().getLogger() << F("Calling setup function...") << endl;
    Interface::get().setupFunction();
    _setupFunctionCalled = true;
  }
}

void BootNormal::_onMqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  Interface::get().connected = false;
  if (!_mqttDisconnectNotified) {
    _statsTimer.reset();
    Interface::get().getLogger() << F("✖ MQTT disconnected") << endl;
    Interface::get().getLogger() << F("Triggering MQTT_DISCONNECTED event...") << endl;
    Interface::get().event.type = HomieEventType::MQTT_DISCONNECTED;
    Interface::get().event.mqttReason = reason;
    Interface::get().eventHandler(Interface::get().event);

    _mqttDisconnectNotified = true;

    if (_mqttOfflineMessageId != 0) {
      _mqttOfflineMessageId = 0;
      Interface::get().getLogger() << F("Triggering READY_TO_SLEEP event...") << endl;
      Interface::get().event.type = HomieEventType::READY_TO_SLEEP;
      Interface::get().eventHandler(Interface::get().event);

      return;
    }

    _mqttConnect();

  } else {
    _mqttTimedRetry.activate();
  }
}

void BootNormal::_onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (total == 0) return;  // no empty message possible

  HomieRange range;
  range.isRange = false;
  range.index = 0;

  // Check for Broadcast first (it does not contain device-id)
  char* broadcast_topic = topic + strlen(Interface::get().getConfig().get().mqtt.baseTopic);
  // Skip devices/${id}/ --- +1 for /
  char* device_topic = broadcast_topic + strlen(Interface::get().getConfig().get().deviceId) + 1;

  // 1. Handle OTA firmware (not copied to payload buffer)
  if (strcmp_P(device_topic, PSTR("$implementation/ota/firmware")) == 0) {  // If this is the OTA firmware
    if (!Interface::get().getConfig().get().ota.enabled) {
      if (index == 0) {
        _publishOtaStatus(403);  // 403 Forbidden
      }
    } else if (!_flaggedForOta) {
      if (index == 0) {
        Interface::get().getLogger() << F("Receiving OTA firmware but not requested, skipping...") << endl;
        _publishOtaStatus(400, PSTR("NOT_REQUESTED"));
      }
    } else {
      if (index == 0) {
        Interface::get().getLogger() << F("↕ OTA started") << endl;
        Interface::get().getLogger() << F("Triggering OTA_STARTED event...") << endl;
        Interface::get().event.type = HomieEventType::OTA_STARTED;
        Interface::get().eventHandler(Interface::get().event);

        // Autodetect if firmware is binary or base64-encoded. ESP firmware always has a magic first byte 0xE9.
        if (*payload == 0xE9) {
          _otaIsBase64 = false;
          Interface::get().getLogger() << F("Firmware is binary") << endl;
        } else {
          // Base64-decode first two bytes. Compare decoded value against magic byte.
          char plain[2];  // need 12 bits
          base64_init_decodestate(&_otaBase64State);
          int l = base64_decode_block(payload, 2, plain, &_otaBase64State);
          if ((l == 1) && (plain[0] == 0xE9)) {
            _otaIsBase64 = true;
            _otaBase64Pads = 0;
            Interface::get().getLogger() << F("Firmware is base64-encoded") << endl;
            if (total % 4) {
              // Base64 encoded length not a multiple of 4 bytes
              _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
              return;
            }

            // Restart base64-decoder
            base64_init_decodestate(&_otaBase64State);
          } else {
            // Bad firmware format
            _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
            return;
          }
        }
        _otaSizeDone = 0;
        _otaSizeTotal = _otaIsBase64 ? base64_decode_expected_len(total) : total;
        bool success = Update.begin(_otaSizeTotal);
        if (!success) {
          // Detected error during begin (e.g. size == 0 or size > space)
          _endOtaUpdate(false, Update.getError());
          return;
        }
      }

      size_t write_len;
      if (_otaIsBase64) {
        // Base64-firmware: Make sure there are no non-base64 characters in the payload.
        // libb64/cdecode.c doesn't ignore such characters if the compiler treats `char`
        // as `unsigned char`.
        size_t bin_len = 0;
        char* p = payload;
        for (size_t i = 0; i < len; i ++) {
          char c = *p++;
          bool b64 = ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '+') || (c == '/');
          if (b64) {
            bin_len++;
          } else if (c == '=') {
            // Ignore "=" padding (but only at the end and only up to 2)
            if (index + i < total - 2) {
              _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
              return;
            }
            // Note the number of pad characters at the end
            _otaBase64Pads++;
          } else {
            // Non-base64 character in firmware
            _endOtaUpdate(false, UPDATE_ERROR_MAGIC_BYTE);
            return;
          }
        }
        if (bin_len > 0) {
          // Decode base64 payload in-place. base64_decode_block() can decode in-place,
          // except for the first two base64-characters which make one binary byte plus
          // 4 extra bits (saved in _otaBase64State). So we "manually" decode the first
          // two characters into a temporary buffer and manually merge that back into
          // the payload. This one is a little tricky, but it saves us from having to
          // dynamically allocate some 800 bytes of memory for every payload chunk.
          size_t dec_len = bin_len > 1 ? 2 : 1;
          char c;
          write_len = (size_t) base64_decode_block(payload, dec_len, &c, &_otaBase64State);
          *payload = c;

          if (bin_len > 1) {
            write_len += (size_t) base64_decode_block((const char*) payload + dec_len, bin_len - dec_len, payload + write_len, &_otaBase64State);
          }
        } else {
          write_len = 0;
        }
      } else {
        // Binary firmware
        write_len = len;
      }
      if (write_len > 0) {
        bool success = Update.write(reinterpret_cast<uint8_t*>(payload), write_len) > 0;
        if (success) {
          // Flash write successful.
          _otaSizeDone += write_len;
          if (_otaIsBase64 && (index + len == total)) {
            // Having received the last chunk of base64 encoded firmware, we can now determine
            // the real size of the binary firmware from the number of padding character ("="):
            // If we have received 1 pad character, real firmware size modulo 3 was 2.
            // If we have received 2 pad characters, real firmware size modulo 3 was 1.
            // Correct the total firmware length accordingly.
            _otaSizeTotal -= _otaBase64Pads;
          }

          String progress(_otaSizeDone);
          progress += F("/");
          progress += _otaSizeTotal;
          Interface::get().getLogger() << F("Receiving OTA firmware (") << progress << F(")...") << endl;
          _publishOtaStatus(206, progress.c_str());  // 206 Partial Content

          //  Done with the update?
          if (index + len == total) {
            // With base64-coded firmware, we may have provided a length off by one or two
            // to Update.begin() because the base64-coded firmware may use padding (one or
            // two "=") at the end. In case of base64, total length was adjusted above.
            // Check the real length here and ask Update::end() to skip this test.
            if ((_otaIsBase64) && (_otaSizeDone != _otaSizeTotal)) {
              _endOtaUpdate(false, UPDATE_ERROR_SIZE);
              return;
            }
            success = Update.end(_otaIsBase64);
            _endOtaUpdate(success, Update.getError());
          }
        } else {
          // Error erasing or writing flash
          _endOtaUpdate(false, Update.getError());
        }
      }
    }
    return;
  }

  // 2. Fill Payload Buffer

  // Reallocate Buffer everytime a new message is received
  if (_mqttPayloadBuffer == nullptr || index == 0) _mqttPayloadBuffer = std::unique_ptr<char[]>(new char[total + 1]);

  // TODO(euphi): Check if buffer size matches payload length
  memcpy(_mqttPayloadBuffer.get() + index, payload, len);

  if (index + len != total) return;  // return if payload buffer is not complete
  _mqttPayloadBuffer.get()[total] = '\0';

  /* Arrived here, the payload is complete */

  if (strcmp_P(device_topic, PSTR("$implementation/ota/checksum")) == 0) {  // If this is the MD5 OTA checksum (32 hex characters)
    Interface::get().getLogger() << F("✴ OTA available (checksum ") << _mqttPayloadBuffer.get() << F(")") << endl;
    if (!Interface::get().getConfig().get().ota.enabled) {
      _publishOtaStatus(403);  // 403 Forbidden
    } else if (strcmp(_mqttPayloadBuffer.get(), _fwChecksum) == 0) {
      _publishOtaStatus(304);  // 304 Not Modified
    } else {
      // 32 hex characters?
      if (strlen(_mqttPayloadBuffer.get()) != 32) {
        // Invalid MD5 number => 400 BAD_CHECKSUM
        _endOtaUpdate(false, UPDATE_ERROR_MD5);
        return;
      }

      for (uint8_t i = 0; i < 32; i++) {
        char c = _mqttPayloadBuffer.get()[i];
        bool valid = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
        if (!valid) {
          _endOtaUpdate(false, UPDATE_ERROR_MD5);
          return;
        }
      }

      _flaggedForOta = true;
      Update.setMD5(_mqttPayloadBuffer.get());
      _publishOtaStatus(202);
    }
    return;
  }

  // 3. Special Functions: $broadcast
  /** TODO(euphi): Homie $broadcast */
  if (strncmp(broadcast_topic, "$broadcast", 10) == 0) {
    broadcast_topic += sizeof("$broadcast");  // move pointer to second char after $broadcast (sizeof counts the \0)
    String broadcastLevel(broadcast_topic);
    Interface::get().getLogger() << F("📢 Calling broadcast handler...") << endl;
    bool handled = Interface::get().broadcastHandler(broadcastLevel, _mqttPayloadBuffer.get());
    if (!handled) {
      Interface::get().getLogger() << F("The following broadcast was not handled:") << endl;
      Interface::get().getLogger() << F("  • Level: ") <<  broadcastLevel << endl;
      Interface::get().getLogger() << F("  • Value: ") << _mqttPayloadBuffer.get() << endl;
    }
    return;
  }

  // 4. Special Functions: $reset
  if (strcmp_P(device_topic, PSTR("$implementation/reset")) == 0 && strcmp(_mqttPayloadBuffer.get(), "true") == 0) {
    Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/reset")), 1, true, "false");
    _flaggedForReset = true;
    Interface::get().getLogger() << F("Flagged for reset by network") << endl;
    return;
  }

  // 5. Special Functions set $config
  if (strcmp_P(device_topic, PSTR("$implementation/config/set")) == 0) {
    Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$implementation/config/set")), 1, true, "");
    if (Interface::get().getConfig().patch(_mqttPayloadBuffer.get())) {
      Interface::get().getLogger() << F("✔ Configuration updated") << endl;
      _flaggedForReboot = true;
      Interface::get().getLogger() << F("Flagged for reboot") << endl;
    } else {
      Interface::get().getLogger() << F("✖ Configuration not updated") << endl;
    }
    return;
  }

  // 6. Determine specific Node
  // Determine if message for our deviceid // [Issue #243]
  const char* messageDeviceId = Interface::get().getConfig().get().deviceId;
  for (uint16_t i = 0; i < strlen(messageDeviceId); i++) {
    if ((broadcast_topic[i] != messageDeviceId[i]) || (broadcast_topic[i] == '/' && messageDeviceId[i] != '\0')) {
      return;
    }
  }

  // Implicit node properties
  device_topic[strlen(device_topic) - 4] = '\0';  // Remove /set
  uint16_t separator = 0;
  for (uint16_t i = 0; i < strlen(device_topic); i++) {
    if (device_topic[i] == '/') {
      separator = i;
      break;
    }
  }
  char* node = device_topic;
  node[separator] = '\0';
  char* property = device_topic + separator + 1;
  HomieNode* homieNode = HomieNode::find(node);
  if (!homieNode) {
    Interface::get().getLogger() << F("Node ") << node << F(" not registered") << endl;
    return;
  }

  int16_t rangeSeparator = -1;
  for (uint16_t i = 0; i < strlen(property); i++) {
    if (property[i] == '_') {
      rangeSeparator = i;
      break;
    }
  }
  if (rangeSeparator != -1) {
    range.isRange = true;
    property[rangeSeparator] = '\0';
    char* rangeIndexStr = property + rangeSeparator + 1;
    String rangeIndexTest = String(rangeIndexStr);
    for (uint8_t i = 0; i < rangeIndexTest.length(); i++) {
      if (!isDigit(rangeIndexTest.charAt(i))) {
        Interface::get().getLogger() << F("Range index ") << rangeIndexStr << F(" is not valid") << endl;
        return;
      }
    }
    range.index = rangeIndexTest.toInt();
  }

  Property* propertyObject = nullptr;
  for (Property* iProperty : homieNode->getProperties()) {
    if (range.isRange) {
      if (iProperty->isRange() && strcmp(property, iProperty->getProperty()) == 0) {
        if (range.index >= iProperty->getLower() && range.index <= iProperty->getUpper()) {
          propertyObject = iProperty;
          break;
        } else {
          Interface::get().getLogger() << F("Range index ") << range.index << F(" is not within the bounds of ") << property << endl;
          return;
        }
      }
    } else if (strcmp(property, iProperty->getProperty()) == 0) {
      propertyObject = iProperty;
      break;
    }
  }

  if (!propertyObject || !propertyObject->isSettable()) {
    Interface::get().getLogger() << F("Node ") << node << F(": ") << property << F(" property not settable") << endl;
    return;
  }

  Interface::get().getLogger() << F("Calling global input handler...") << endl;
  bool handled = Interface::get().globalInputHandler(*homieNode, String(property), range, String(_mqttPayloadBuffer.get()));
  if (handled) return;

  Interface::get().getLogger() << F("Calling node input handler...") << endl;
  handled = homieNode->handleInput(String(property), range, String(_mqttPayloadBuffer.get()));
  if (handled) return;

  Interface::get().getLogger() << F("Calling property input handler...") << endl;
  handled = propertyObject->getInputHandler()(range, String(_mqttPayloadBuffer.get()));

  if (!handled) {
    Interface::get().getLogger() << F("No handlers handled the following packet:") << endl;
    Interface::get().getLogger() << F("  • Node ID: ") << node << endl;
    Interface::get().getLogger() << F("  • Property: ") << property << endl;
    Interface::get().getLogger() << F("  • Is range? ");
    if (range.isRange) {
      Interface::get().getLogger() << F("yes (") << range.index << F(")") << endl;
    } else {
      Interface::get().getLogger() << F("no") << endl;
    }
    Interface::get().getLogger() << F("  • Value: ") << _mqttPayloadBuffer.get() << endl;
  }
}

void BootNormal::_onMqttPublish(uint16_t id) {
  Interface::get().event.type = HomieEventType::MQTT_PACKET_ACKNOWLEDGED;
  Interface::get().event.packetId = id;
  Interface::get().eventHandler(Interface::get().event);

  if (Interface::get().flaggedForSleep && id == _mqttOfflineMessageId) {
    Interface::get().getLogger() << F("Offline message acknowledged. Disconnecting MQTT...") << endl;
    Interface::get().getMqttClient().disconnect();
  }
}

void BootNormal::_handleReset() {
  if (Interface::get().reset.enabled) {
    _resetDebouncer.update();

    if (_resetDebouncer.read() == Interface::get().reset.triggerState) {
      _flaggedForReset = true;
      Interface::get().getLogger() << F("Flagged for reset by pin") << endl;
    }
  }

  if (Interface::get().reset.flaggedBySketch) {
    _flaggedForReset = true;
    Interface::get().getLogger() << F("Flagged for reset by sketch") << endl;
  }
}

void BootNormal::setup() {
  Boot::setup();

  Update.runAsync(true);

  if (Interface::get().led.enabled) Interface::get().getBlinker().start(LED_WIFI_DELAY);

  // Generate topic buffer
  size_t baseTopicLength = strlen(Interface::get().getConfig().get().mqtt.baseTopic) + strlen(Interface::get().getConfig().get().deviceId);
  size_t longestSubtopicLength = 29 + 1;  // /$implementation/ota/firmware
  for (HomieNode* iNode : HomieNode::nodes) {
    size_t nodeMaxTopicLength = 1 + strlen(iNode->getId()) + 12 + 1;  // /id/$properties
    if (nodeMaxTopicLength > longestSubtopicLength) longestSubtopicLength = nodeMaxTopicLength;

    for (Property* iProperty : iNode->getProperties()) {
      size_t propertyMaxTopicLength = 1 + strlen(iNode->getId()) + 1 + strlen(iProperty->getProperty()) + 1;
      if (iProperty->isSettable()) propertyMaxTopicLength += 4;  // /set

      if (propertyMaxTopicLength > longestSubtopicLength) longestSubtopicLength = propertyMaxTopicLength;
    }
  }
  _mqttTopic = std::unique_ptr<char[]>(new char[baseTopicLength + longestSubtopicLength]);

  _wifiGotIpHandler = WiFi.onStationModeGotIP(std::bind(&BootNormal::_onWifiGotIp, this, std::placeholders::_1));
  _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&BootNormal::_onWifiDisconnected, this, std::placeholders::_1));

  Interface::get().getMqttClient().onConnect(std::bind(&BootNormal::_onMqttConnected, this));
  Interface::get().getMqttClient().onDisconnect(std::bind(&BootNormal::_onMqttDisconnected, this, std::placeholders::_1));
  Interface::get().getMqttClient().onMessage(std::bind(&BootNormal::_onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  Interface::get().getMqttClient().onPublish(std::bind(&BootNormal::_onMqttPublish, this, std::placeholders::_1));

  Interface::get().getMqttClient().setServer(Interface::get().getConfig().get().mqtt.server.host, Interface::get().getConfig().get().mqtt.server.port);
  Interface::get().getMqttClient().setMaxTopicLength(MAX_MQTT_TOPIC_LENGTH);
  _mqttClientId = std::unique_ptr<char[]>(new char[strlen(Interface::get().brand) + 1 + strlen(Interface::get().getConfig().get().deviceId) + 1]);
  strcpy(_mqttClientId.get(), Interface::get().brand);
  strcat_P(_mqttClientId.get(), PSTR("-"));
  strcat(_mqttClientId.get(), Interface::get().getConfig().get().deviceId);
  Interface::get().getMqttClient().setClientId(_mqttClientId.get());
  char* mqttWillTopic = _prefixMqttTopic(PSTR("/$online"));
  _mqttWillTopic = std::unique_ptr<char[]>(new char[strlen(mqttWillTopic) + 1]);
  memcpy(_mqttWillTopic.get(), mqttWillTopic, strlen(mqttWillTopic) + 1);
  Interface::get().getMqttClient().setWill(_mqttWillTopic.get(), 1, true, "false");

  if (Interface::get().getConfig().get().mqtt.auth) Interface::get().getMqttClient().setCredentials(Interface::get().getConfig().get().mqtt.username, Interface::get().getConfig().get().mqtt.password);


  if (Interface::get().reset.enabled) {
    pinMode(Interface::get().reset.triggerPin, INPUT_PULLUP);

    _resetDebouncer.attach(Interface::get().reset.triggerPin);
    _resetDebouncer.interval(Interface::get().reset.triggerTime);
  }

  Interface::get().getConfig().log();

  for (HomieNode* iNode : HomieNode::nodes) {
    iNode->setup();
  }

  _wifiConnect();
}

void BootNormal::loop() {
  Boot::loop();

  _handleReset();

  if (_mqttTimedRetry.check()) {
    _mqttConnect();
  }

  if (_flaggedForReset && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;
    Interface::get().getConfig().erase();
    Interface::get().getLogger() << F("Configuration erased") << endl;

    Interface::get().getLogger() << F("Triggering ABOUT_TO_RESET event...") << endl;
    Interface::get().event.type = HomieEventType::ABOUT_TO_RESET;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().getLogger() << F("↻ Rebooting into config mode...") << endl;
    Serial.flush();
    ESP.restart();
  }

  if (_flaggedForReboot && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;

    Interface::get().getLogger() << F("↻ Rebooting...") << endl;
    Serial.flush();
    ESP.restart();
  }

  if (Interface::get().connected) {
    if (_mqttOfflineMessageId == 0 && Interface::get().flaggedForSleep) {
      Interface::get().getLogger() << F("Device in preparation to sleep...") << endl;
      _mqttOfflineMessageId = Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$online")), 1, true, "false");
    }

    if (_statsTimer.check()) {
      uint8_t quality = Helpers::rssiToPercentage(WiFi.RSSI());
      char qualityStr[3 + 1];
      itoa(quality, qualityStr, 10);
      Interface::get().getLogger() << F("〽 Sending statistics...") << endl;
      Interface::get().getLogger() << F("  • Wi-Fi signal quality: ") << qualityStr << F("%") << endl;
      Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats/signal")), 1, true, qualityStr);

      _uptime.update();
      char uptimeStr[20 + 1];
      itoa(_uptime.getSeconds(), uptimeStr, 10);
      Interface::get().getLogger() << F("  • Uptime: ") << uptimeStr << F("s") << endl;
      Interface::get().getMqttClient().publish(_prefixMqttTopic(PSTR("/$stats/uptime")), 1, true, uptimeStr);
      _statsTimer.tick();
    }

    Interface::get().loopFunction();

    for (HomieNode* iNode : HomieNode::nodes) {
      iNode->loop();
    }
  }
}

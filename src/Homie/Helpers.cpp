#include "Helpers.hpp"

using namespace HomieInternals;

char Helpers::_deviceId[] = ""; // need to define the static variable

void Helpers::generateDeviceId() {
  char flashChipId[6 + 1];
  sprintf(flashChipId, "%06x", ESP.getFlashChipId());

  sprintf(Helpers::_deviceId, "%06x%s", ESP.getChipId(), flashChipId + strlen(flashChipId) - 2);
}

const char* Helpers::getDeviceId() {
  return Helpers::_deviceId;
}

MdnsQueryResult Helpers::mdnsQuery(const char* service) {
  MdnsQueryResult result;
  result.success = false;
  int n = MDNS.queryService(service, "tcp");
  if (n == 0) {
    return result;
  } else {
    result.success = true;
    result.ip = MDNS.IP(0);
    result.port = MDNS.port(0);
  }

  return result;
}

ConfigValidationResult Helpers::validateConfig(const JsonObject& object) {
  ConfigValidationResult result;
  result = _validateConfigRoot(object);
  if (!result.valid) return result;
  result = _validateConfigWifi(object);
  if (!result.valid) return result;
  result = _validateConfigMqtt(object);
  if (!result.valid) return result;
  result = _validateConfigOta(object);
  if (!result.valid) return result;

  result.valid = true;
  result.reason = nullptr;
  return result;
}

ConfigValidationResult Helpers::_validateConfigRoot(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;
  result.reason = nullptr;
  if (!object.containsKey("name") || !object["name"].is<const char*>()) {
    result.reason = F("name is not a string");
    return result;
  }
  if (strlen(object["name"]) + 1 > MAX_FRIENDLY_NAME_LENGTH) {
    result.reason = F("name is too long");
    return result;
  }
  if (object.containsKey("device_id")) {
    if (!object["device_id"].is<const char*>()) {
      result.reason = F("device_id is not a string");
      return result;
    }
    if (strlen(object["device_id"]) + 1 > MAX_DEVICE_ID_LENGTH) {
      result.reason = F("device_id is too long");
      return result;
    }
  }

  const char* name = object["name"];

  if (strcmp_P(name, PSTR("")) == 0) {
    result.reason = F("name is empty");
    return result;
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Helpers::_validateConfigWifi(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;
  result.reason = nullptr;

  if (!object.containsKey("wifi") || !object["wifi"].is<JsonObject&>()) {
    result.reason = F("wifi is not an object");
    return result;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("ssid") || !object["wifi"]["ssid"].is<const char*>()) {
    result.reason = F("wifi.ssid is not a string");
    return result;
  }
  if (strlen(object["wifi"]["ssid"]) + 1 > MAX_WIFI_SSID_LENGTH) {
    result.reason = F("wifi.ssid is too long");
    return result;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("password") || !object["wifi"]["password"].is<const char*>()) {
    result.reason = F("wifi.password is not a string");
    return result;
  }
  if (strlen(object["wifi"]["password"]) + 1 > MAX_WIFI_PASSWORD_LENGTH) {
    result.reason = F("wifi.password is too long");
    return result;
  }

  const char* wifiSsid = object["wifi"]["ssid"];
  if (strcmp_P(wifiSsid, PSTR("")) == 0) {
    result.reason = F("wifi.ssid is empty");
    return result;
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Helpers::_validateConfigMqtt(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;
  result.reason = nullptr;

  if (!object.containsKey("mqtt") || !object["mqtt"].is<JsonObject&>()) {
    result.reason = F("mqtt is not an object");
    return result;
  }
  bool mdns = false;
  if (object["mqtt"].as<JsonObject&>().containsKey("mdns")) {
    if (!object["mqtt"]["mdns"].is<const char*>()) {
      result.reason = F("mqtt.mdns is not a string");
      return result;
    }
    if (strlen(object["mqtt"]["mdns"]) + 1 > MAX_HOSTNAME_LENGTH) {
      result.reason = F("mqtt.mdns is too long");
      return result;
    }
    mdns = true;
  } else {
    if (!object["mqtt"].as<JsonObject&>().containsKey("host") || !object["mqtt"]["host"].is<const char*>()) {
      result.reason = F("mqtt.host is not a string");
      return result;
    }
    if (strlen(object["mqtt"]["host"]) + 1 > MAX_HOSTNAME_LENGTH) {
      result.reason = F("mqtt.host is too long");
      return result;
    }
    if (object["mqtt"].as<JsonObject&>().containsKey("port") && !object["mqtt"]["port"].is<unsigned int>()) {
      result.reason = F("mqtt.port is not an unsigned integer");
      return result;
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("base_topic")) {
    if (!object["mqtt"]["base_topic"].is<const char*>()) {
      result.reason = F("mqtt.base_topic is not a string");
      return result;
    }

    if (strlen(object["mqtt"]["base_topic"]) + 1 > MAX_MQTT_BASE_TOPIC_LENGTH) {
      result.reason = F("mqtt.base_topic is too long");
      return result;
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("auth")) {
    if (!object["mqtt"]["auth"].is<bool>()) {
      result.reason = F("mqtt.auth is not a boolean");
      return result;
    }

    if (object["mqtt"]["auth"]) {
      if (!object["mqtt"].as<JsonObject&>().containsKey("username") || !object["mqtt"]["username"].is<const char*>()) {
        result.reason = F("mqtt.username is not a string");
        return result;
      }
      if (strlen(object["mqtt"]["username"]) + 1 > MAX_MQTT_CREDS_LENGTH) {
        result.reason = F("mqtt.username is too long");
        return result;
      }
      if (!object["mqtt"].as<JsonObject&>().containsKey("password") || !object["mqtt"]["password"].is<const char*>()) {
        result.reason = F("mqtt.password is not a string");
        return result;
      }
      if (strlen(object["mqtt"]["password"]) + 1 > MAX_MQTT_CREDS_LENGTH) {
        result.reason = F("mqtt.password is too long");
        return result;
      }
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("ssl")) {
    if (!object["mqtt"]["ssl"].is<bool>()) {
      result.reason = F("mqtt.ssl is not a boolean");
      return result;
    }

    if (object["mqtt"]["ssl"]) {
      if (object["mqtt"].as<JsonObject&>().containsKey("fingerprint") && !object["mqtt"]["fingerprint"].is<const char*>()) {
        result.reason = F("mqtt.fingerprint is not a string");
        return result;
      }
    }
  }

  if (mdns) {
    const char* mdnsService = object["mqtt"]["mdns"];
    if (strcmp_P(mdnsService, PSTR("")) == 0) {
      result.reason = F("mqtt.mdns is empty");
      return result;
    }
  } else {
    const char* host = object["mqtt"]["host"];
    if (strcmp_P(host, PSTR("")) == 0) {
      result.reason = F("mqtt.host is empty");
      return result;
    }
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Helpers::_validateConfigOta(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;
  result.reason = nullptr;

  if (!object.containsKey("ota") || !object["ota"].is<JsonObject&>()) {
    result.reason = F("ota is not an object");
    return result;
  }
  if (!object["ota"].as<JsonObject&>().containsKey("enabled") || !object["ota"]["enabled"].is<bool>()) {
    result.reason = F("ota.enabled is not a boolean");
    return result;
  }
  if (object["ota"]["enabled"]) {
    if (object["ota"].as<JsonObject&>().containsKey("mdns")) {
      if (!object["ota"]["mdns"].is<const char*>()) {
        result.reason = F("ota.mdns is not a string");
        return result;
      }
      if (strlen(object["ota"]["mdns"]) + 1 > MAX_HOSTNAME_LENGTH) {
        result.reason = F("ota.mdns is too long");
        return result;
      }
    } else {
      if (object["ota"].as<JsonObject&>().containsKey("host")) {
        if (!object["ota"]["host"].is<const char*>()) {
          result.reason = F("ota.host is not a string");
          return result;
        }
        if (strlen(object["ota"]["host"]) + 1 > MAX_HOSTNAME_LENGTH) {
          result.reason = F("ota.host is too long");
          return result;
        }
      }
      if (object["ota"].as<JsonObject&>().containsKey("port") && !object["ota"]["port"].is<unsigned int>()) {
        result.reason = F("ota.port is not an unsigned integer");
        return result;
      }
    }
    if (object["ota"].as<JsonObject&>().containsKey("path")) {
      if (!object["ota"]["path"].is<const char*>()) {
        result.reason = F("ota.path is not a string");
        return result;
      }
      if (strlen(object["ota"]["path"]) + 1 > MAX_OTA_PATH_LENGTH) {
        result.reason = F("ota.path is too long");
        return result;
      }
    }
    if (object["ota"].as<JsonObject&>().containsKey("ssl")) {
      if (!object["ota"]["ssl"].is<bool>()) {
        result.reason = F("ota.ssl is not a boolean");
        return result;
      }

      if (object["ota"]["ssl"]) {
        if (object["ota"].as<JsonObject&>().containsKey("fingerprint") && !object["ota"]["fingerprint"].is<const char*>()) {
          result.reason = F("ota.fingerprint is not a string");
          return result;
        }
      }
    }
  }

  result.valid = true;
  return result;
}

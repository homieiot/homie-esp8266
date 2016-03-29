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

bool Helpers::validateConfig(const JsonObject& object) {
  if (!_validateConfigRoot(object)) return false;
  if (!_validateConfigWifi(object)) return false;
  if (!_validateConfigMqtt(object)) return false;
  if (!_validateConfigOta(object)) return false;

  return true;
}

bool Helpers::_validateConfigRoot(const JsonObject& object) {
  if (!object.containsKey("name") || !object["name"].is<const char*>()) {
    Logger.logln(F("✖ name is not a string"));
    return false;
  }
  if (strlen(object["name"]) + 1 > MAX_FRIENDLY_NAME_LENGTH) {
    Logger.logln(F("✖ name is too long"));
    return false;
  }
  if (object.containsKey("device_id")) {
    if (!object["device_id"].is<const char*>()) {
      Logger.logln(F("✖ device_id is not a string"));
      return false;
    }
    if (strlen(object["device_id"]) + 1 > MAX_DEVICE_ID_LENGTH) {
      Logger.logln(F("✖ device_id is too long"));
      return false;
    }
  }

  const char* name = object["name"];

  if (strcmp_P(name, PSTR("")) == 0) {
    Logger.logln(F("✖ name is empty"));
    return false;
  }

  return true;
}

bool Helpers::_validateConfigWifi(const JsonObject& object) {
  if (!object.containsKey("wifi") || !object["wifi"].is<JsonObject&>()) {
    Logger.logln(F("✖ wifi is not an object"));
    return false;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("ssid") || !object["wifi"]["ssid"].is<const char*>()) {
    Logger.logln(F("✖ wifi.ssid is not a string"));
    return false;
  }
  if (strlen(object["wifi"]["ssid"]) + 1 > MAX_WIFI_SSID_LENGTH) {
    Logger.logln(F("✖ wifi.ssid is too long"));
    return false;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("password") || !object["wifi"]["password"].is<const char*>()) {
    Logger.logln(F("✖ wifi.password is not a string"));
    return false;
  }
  if (strlen(object["wifi"]["password"]) + 1 > MAX_WIFI_PASSWORD_LENGTH) {
    Logger.logln(F("✖ wifi.password is too long"));
    return false;
  }

  const char* wifiSsid = object["wifi"]["ssid"];
  if (strcmp_P(wifiSsid, PSTR("")) == 0) {
    Logger.logln(F("✖ wifi.ssid is empty"));
    return false;
  }

  return true;
}

bool Helpers::_validateConfigMqtt(const JsonObject& object) {
  if (!object.containsKey("mqtt") || !object["mqtt"].is<JsonObject&>()) {
    Logger.logln(F("✖ mqtt is not an object"));
    return false;
  }
  bool mdns = false;
  if (object["mqtt"].as<JsonObject&>().containsKey("mdns")) {
    if (!object["mqtt"]["mdns"].is<const char*>()) {
      Logger.logln(F("✖ mqtt.mdns is not a string"));
      return false;
    }
    if (strlen(object["mqtt"]["mdns"]) + 1 > MAX_HOSTNAME_LENGTH) {
      Logger.logln(F("✖ mqtt.mdns is too long"));
      return false;
    }
    mdns = true;
  } else {
    if (!object["mqtt"].as<JsonObject&>().containsKey("host") || !object["mqtt"]["host"].is<const char*>()) {
      Logger.logln(F("✖ mqtt.host is not a string"));
      return false;
    }
    if (strlen(object["mqtt"]["host"]) + 1 > MAX_HOSTNAME_LENGTH) {
      Logger.logln(F("✖ mqtt.host is too long"));
      return false;
    }
    if (object["mqtt"].as<JsonObject&>().containsKey("port") && !object["mqtt"]["port"].is<unsigned int>()) {
      Logger.logln(F("✖ mqtt.port is not an unsigned integer"));
      return false;
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("base_topic")) {
    if (!object["mqtt"]["base_topic"].is<const char*>()) {
      Logger.logln(F("✖ mqtt.base_topic is not a string"));
      return false;
    }

    if (strlen(object["mqtt"]["base_topic"]) + 1 > MAX_MQTT_BASE_TOPIC_LENGTH) {
      Logger.logln(F("✖ mqtt.base_topic is too long"));
      return false;
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("auth")) {
    if (!object["mqtt"]["auth"].is<bool>()) {
      Logger.logln(F("✖ mqtt.auth is not a boolean"));
      return false;
    }

    if (object["mqtt"]["auth"]) {
      if (!object["mqtt"].as<JsonObject&>().containsKey("username") || !object["mqtt"]["username"].is<const char*>()) {
        Logger.logln(F("✖ mqtt.username is not a string"));
        return false;
      }
      if (strlen(object["mqtt"]["username"]) + 1 > MAX_MQTT_CREDS_LENGTH) {
        Logger.logln(F("✖ mqtt.username is too long"));
        return false;
      }
      if (!object["mqtt"].as<JsonObject&>().containsKey("password") || !object["mqtt"]["password"].is<const char*>()) {
        Logger.logln(F("✖ mqtt.password is not a string"));
        return false;
      }
      if (strlen(object["mqtt"]["password"]) + 1 > MAX_MQTT_CREDS_LENGTH) {
        Logger.logln(F("✖ mqtt.password is too long"));
        return false;
      }
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("ssl")) {
    if (!object["mqtt"]["ssl"].is<bool>()) {
      Logger.logln(F("✖ mqtt.ssl is not a boolean"));
      return false;
    }

    if (object["mqtt"]["ssl"]) {
      if (object["mqtt"].as<JsonObject&>().containsKey("fingerprint") && !object["mqtt"]["fingerprint"].is<const char*>()) {
        Logger.logln(F("✖ mqtt.fingerprint is not a string"));
        return false;
      }
    }
  }

  if (mdns) {
    const char* mdnsService = object["mqtt"]["mdns"];
    if (strcmp_P(mdnsService, PSTR("")) == 0) {
      Logger.logln(F("✖ mqtt.mdns is empty"));
      return false;
    }
  } else {
    const char* host = object["mqtt"]["host"];
    if (strcmp_P(host, PSTR("")) == 0) {
      Logger.logln(F("✖ mqtt.host is empty"));
      return false;
    }
  }

  return true;
}

bool Helpers::_validateConfigOta(const JsonObject& object) {
  if (!object.containsKey("ota") || !object["ota"].is<JsonObject&>()) {
    Logger.logln(F("✖ ota is not an object"));
    return false;
  }
  if (!object["ota"].as<JsonObject&>().containsKey("enabled") || !object["ota"]["enabled"].is<bool>()) {
    Logger.logln(F("✖ ota.enabled is not a boolean"));
    return false;
  }
  if (object["ota"]["enabled"]) {
    if (object["ota"].as<JsonObject&>().containsKey("mdns")) {
      if (!object["ota"]["mdns"].is<const char*>()) {
        Logger.logln(F("✖ ota.mdns is not a string"));
        return false;
      }
      if (strlen(object["ota"]["mdns"]) + 1 > MAX_HOSTNAME_LENGTH) {
        Logger.logln(F("✖ ota.mdns is too long"));
        return false;
      }
    } else {
      if (object["ota"].as<JsonObject&>().containsKey("host")) {
        if (!object["ota"]["host"].is<const char*>()) {
          Logger.logln(F("✖ ota.host is not a string"));
          return false;
        }
        if (strlen(object["ota"]["host"]) + 1 > MAX_HOSTNAME_LENGTH) {
          Logger.logln(F("✖ ota.host is too long"));
          return false;
        }
      }
      if (object["ota"].as<JsonObject&>().containsKey("port") && !object["ota"]["port"].is<unsigned int>()) {
        Logger.logln(F("✖ ota.port is not an unsigned integer"));
        return false;
      }
    }
    if (object["ota"].as<JsonObject&>().containsKey("path")) {
      if (!object["ota"]["path"].is<const char*>()) {
        Logger.logln(F("✖ ota.path is not a string"));
        return false;
      }
      if (strlen(object["ota"]["path"]) + 1 > MAX_OTA_PATH_LENGTH) {
        Logger.logln(F("✖ ota.path is too long"));
        return false;
      }
    }
    if (object["ota"].as<JsonObject&>().containsKey("ssl")) {
      if (!object["ota"]["ssl"].is<bool>()) {
        Logger.logln(F("✖ ota.ssl is not a boolean"));
        return false;
      }

      if (object["ota"]["ssl"]) {
        if (object["ota"].as<JsonObject&>().containsKey("fingerprint") && !object["ota"]["fingerprint"].is<const char*>()) {
          Logger.logln(F("✖ ota.fingerprint is not a string"));
          return false;
        }
      }
    }
  }

  return true;
}

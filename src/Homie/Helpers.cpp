#include "Helpers.hpp"

using namespace HomieInternals;

HelpersClass::HelpersClass() {
  char flash_chip_id[6 + 1];
  sprintf(flash_chip_id, "%06x", ESP.getFlashChipId());

  sprintf(this->_deviceId, "%06x%s", ESP.getChipId(), flash_chip_id + strlen(flash_chip_id) - 2);
}

const char* HelpersClass::getDeviceId() {
  return this->_deviceId;
}

bool HelpersClass::validateConfig(JsonObject& object) {
  if (!object.containsKey("name") || !object["name"].is<const char*>()) {
    Logger.logln(F("✖ name is not a string"));
    return false;
  }

  if (!object.containsKey("wifi") || !object["wifi"].is<JsonObject&>()) {
    Logger.logln(F("✖ wifi is not an object"));
    return false;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("ssid") || !object["wifi"]["ssid"].is<const char*>()) {
    Logger.logln(F("✖ wifi.ssid is not a string"));
    return false;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("password") || !object["wifi"]["password"].is<const char*>()) {
    Logger.logln(F("✖ wifi.password is not a string"));
    return false;
  }

  if (!object.containsKey("mqtt") || !object["mqtt"].is<JsonObject&>()) {
    Logger.logln(F("✖ mqtt is not an object"));
    return false;
  }
  bool mqttMdns = false;
  if (object["mqtt"].as<JsonObject&>().containsKey("mdns")) {
    if (!object["mqtt"]["mdns"].is<const char*>()) {
      Logger.logln(F("✖ mqtt.mdns is not a string"));
      return false;
    }
    mqttMdns = true;
  } else {
    if (!object["mqtt"].as<JsonObject&>().containsKey("host") || !object["mqtt"]["host"].is<const char*>()) {
      Logger.logln(F("✖ mqtt.host is not a string"));
      return false;
    }
    if (object["mqtt"].as<JsonObject&>().containsKey("port") && !object["mqtt"]["port"].is<uint16_t>()) {
      Logger.logln(F("✖ mqtt.port is not an unsigned integer"));
      return false;
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("base_topic") && !object["mqtt"]["base_topic"].is<const char*>()) {
    Logger.logln(F("✖ mqtt.base_topic is not a string"));
    return false;
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
      if (!object["mqtt"].as<JsonObject&>().containsKey("password") || !object["mqtt"]["password"].is<const char*>()) {
        Logger.logln(F("✖ mqtt.password is not a string"));
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

  if (!object.containsKey("ota") || !object["ota"].is<JsonObject&>()) {
    Logger.logln(F("✖ ota is not an object"));
    return false;
  }
  if (!object["ota"].as<JsonObject&>().containsKey("enabled") || !object["ota"]["enabled"].is<bool>()) {
    Logger.logln(F("✖ ota.enabled is not a boolean"));
    return false;
  }
  bool otaMdns = false;
  if (object["ota"]["enabled"]) {
    if (object["ota"].as<JsonObject&>().containsKey("mdns")) {
      if (!object["ota"]["mdns"].is<const char*>()) {
        Logger.logln(F("✖ ota.mdns is not a string"));
        return false;
      }
      otaMdns = true;
    } else {
      if (object["ota"].as<JsonObject&>().containsKey("host") && !object["ota"]["host"].is<const char*>()) {
        Logger.logln(F("✖ ota.host is not a string"));
        return false;
      }
      if (object["ota"].as<JsonObject&>().containsKey("port") && !object["ota"]["port"].is<uint16_t>()) {
        Logger.logln(F("✖ ota.port is not an unsigned integer"));
        return false;
      }
    }
    if (object["ota"].as<JsonObject&>().containsKey("path") && !object["ota"]["path"].is<const char*>()) {
      Logger.logln(F("✖ ota.path is not a string"));
      return false;
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

  const char* name = object["name"];
  const char* wifiSsid = object["wifi"]["ssid"];

  if (strcmp_P(name, PSTR("")) == 0) {
    Logger.logln(F("✖ name is empty"));
    return false;
  }
  if (strcmp_P(wifiSsid, PSTR("")) == 0) {
    Logger.logln(F("✖ wifi.ssid is empty"));
    return false;
  }
  if (mqttMdns) {
    const char* mqttMdnsService = object["mqtt"]["mdns"];
    if (strcmp_P(mqttMdnsService, PSTR("")) == 0) {
      Logger.logln(F("✖ mqtt.mdns is empty"));
      return false;
    }
  } else {
    const char* mqttHost = object["mqtt"]["host"];
    if (strcmp_P(mqttHost, PSTR("")) == 0) {
      Logger.logln(F("✖ mqtt.host is empty"));
      return false;
    }
  }

  return true;
}

HelpersClass HomieInternals::Helpers;

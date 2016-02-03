#include "Helpers.hpp"

using namespace HomieInternals;


String Helpers::getDeviceId() {
  char chip_id[6 + 1];
  sprintf(chip_id, "%06x", ESP.getChipId());
  char flash_chip_id[6 + 1];
  sprintf(flash_chip_id, "%06x", ESP.getFlashChipId());

  String truncated_flash_id = String(flash_chip_id);
  truncated_flash_id = truncated_flash_id.substring(4);

  String device_id = String(chip_id);
  device_id += truncated_flash_id;

  return device_id;
}

bool Helpers::validateConfig(JsonObject& object) {
  if (!object.containsKey("name") || !object["name"].is<const char*>()) {
    Logger.logln("✖ name is not a string");
    return false;
  }

  if (!object.containsKey("wifi") || !object["wifi"].is<JsonObject&>()) {
    Logger.logln("✖ wifi is not an object");
    return false;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("ssid") || !object["wifi"]["ssid"].is<const char*>()) {
    Logger.logln("✖ wifi.ssid is not a string");
    return false;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("password") || !object["wifi"]["password"].is<const char*>()) {
    Logger.logln("✖ wifi.password is not a string");
    return false;
  }

  if (!object.containsKey("mqtt") || !object["mqtt"].is<JsonObject&>()) {
    Logger.logln("✖ mqtt is not an object");
    return false;
  }
  if (!object["mqtt"].as<JsonObject&>().containsKey("host") || !object["mqtt"]["host"].is<const char*>()) {
    Logger.logln("✖ mqtt.host is not a string");
    return false;
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("port") && !object["mqtt"]["port"].is<uint16_t>()) {
    Logger.logln("✖ mqtt.port is not an unsigned integer");
    return false;
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("auth")) {
    if (!object["mqtt"]["auth"].is<bool>()) {
      Logger.logln("✖ mqtt.auth is not a boolean");
      return false;
    }

    if (object["mqtt"]["auth"]) {
      if (!object["mqtt"].as<JsonObject&>().containsKey("username") || !object["mqtt"]["username"].is<const char*>()) {
        Logger.logln("✖ mqtt.username is not a string");
        return false;
      }
      if (!object["mqtt"].as<JsonObject&>().containsKey("password") || !object["mqtt"]["password"].is<const char*>()) {
        Logger.logln("✖ mqtt.password is not a string");
        return false;
      }
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("ssl")) {
    if (!object["mqtt"]["ssl"].is<bool>()) {
      Logger.logln("✖ mqtt.ssl is not a boolean");
      return false;
    }

    if (object["mqtt"]["ssl"]) {
      if (object["mqtt"].as<JsonObject&>().containsKey("fingerprint") && !object["mqtt"]["fingerprint"].is<const char*>()) {
        Logger.logln("✖ mqtt.fingerprint is not a string");
        return false;
      }
    }
  }

  if (!object.containsKey("ota") || !object["ota"].is<JsonObject&>()) {
    Logger.logln("✖ ota is not an object");
    return false;
  }
  if (!object["ota"].as<JsonObject&>().containsKey("enabled") || !object["ota"]["enabled"].is<bool>()) {
    Logger.logln("✖ ota.enabled is not a boolean");
    return false;
  }
  if (object["ota"]["enabled"]) {
    if (object["ota"].as<JsonObject&>().containsKey("host") && !object["ota"]["host"].is<const char*>()) {
      Logger.logln("✖ ota.host is not a string");
      return false;
    }
    if (object["ota"].as<JsonObject&>().containsKey("port") && !object["ota"]["port"].is<uint16_t>()) {
      Logger.logln("✖ ota.port is not an unsigned integer");
      return false;
    }
    if (object["ota"].as<JsonObject&>().containsKey("path") && !object["ota"]["path"].is<const char*>()) {
      Logger.logln("✖ ota.path is not a string");
      return false;
    }
    if (object["ota"].as<JsonObject&>().containsKey("ssl")) {
      if (!object["ota"]["ssl"].is<bool>()) {
        Logger.logln("✖ ota.ssl is not a boolean");
        return false;
      }

      if (object["ota"]["ssl"]) {
        if (object["ota"].as<JsonObject&>().containsKey("fingerprint") && !object["ota"]["fingerprint"].is<const char*>()) {
          Logger.logln("✖ ota.fingerprint is not a string");
          return false;
        }
      }
    }
  }

  const char* name = object["name"];
  const char* wifi_ssid = object["wifi"]["ssid"];
  const char* mqtt_host = object["mqtt"]["host"];

  if (strcmp(name, "") == 0) {
    Logger.logln("✖ name is empty");
    return false;
  }
  if (strcmp(wifi_ssid, "") == 0) {
    Logger.logln("✖ wifi.ssid is empty");
    return false;
  }
  if (strcmp(mqtt_host, "") == 0) {
    Logger.logln("✖ mqtt.host is empty");
    return false;
  }

  return true;
}

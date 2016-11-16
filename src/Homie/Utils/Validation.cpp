#include "Validation.hpp"

using namespace HomieInternals;

ConfigValidationResult Validation::validateConfig(const JsonObject& object) {
  ConfigValidationResult result;
  result = _validateConfigRoot(object);
  if (!result.valid) return result;
  result = _validateConfigWifi(object);
  if (!result.valid) return result;
  result = _validateConfigMqtt(object);
  if (!result.valid) return result;
  result = _validateConfigOta(object);
  if (!result.valid) return result;
  result = _validateConfigSettings(object);
  if (!result.valid) return result;

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigRoot(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;
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

ConfigValidationResult Validation::_validateConfigWifi(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;

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
  if (object["wifi"]["password"] && strlen(object["wifi"]["password"]) + 1 > MAX_WIFI_PASSWORD_LENGTH) {
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

ConfigValidationResult Validation::_validateConfigMqtt(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;

  if (!object.containsKey("mqtt") || !object["mqtt"].is<JsonObject&>()) {
    result.reason = F("mqtt is not an object");
    return result;
  }
  if (!object["mqtt"].as<JsonObject&>().containsKey("host") || !object["mqtt"]["host"].is<const char*>()) {
    result.reason = F("mqtt.host is not a string");
    return result;
  }
  if (strlen(object["mqtt"]["host"]) + 1 > MAX_HOSTNAME_LENGTH) {
    result.reason = F("mqtt.host is too long");
    return result;
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("port") && !object["mqtt"]["port"].is<uint16_t>()) {
    result.reason = F("mqtt.port is not an integer");
    return result;
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

  const char* host = object["mqtt"]["host"];
  if (strcmp_P(host, PSTR("")) == 0) {
    result.reason = F("mqtt.host is empty");
    return result;
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigOta(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;

  if (!object.containsKey("ota") || !object["ota"].is<JsonObject&>()) {
    result.reason = F("ota is not an object");
    return result;
  }
  if (!object["ota"].as<JsonObject&>().containsKey("enabled") || !object["ota"]["enabled"].is<bool>()) {
    result.reason = F("ota.enabled is not a boolean");
    return result;
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigSettings(const JsonObject& object) {
  ConfigValidationResult result;
  result.valid = false;

  StaticJsonBuffer<0> emptySettingsBuffer;

  JsonObject* settingsObject = &(emptySettingsBuffer.createObject());

  if (object.containsKey("settings") && object["settings"].is<JsonObject&>()) {
    settingsObject = &(object["settings"].as<JsonObject&>());
  }

  for (IHomieSetting* iSetting : IHomieSetting::settings) {
    if (iSetting->isBool()) {
      HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);

      if (settingsObject->containsKey(setting->getName())) {
        if (!(*settingsObject)[setting->getName()].is<bool>()) {
          result.reason = String(setting->getName());
          result.reason.concat(F(" setting is not a boolean"));
          return result;
        } else if (!setting->validate((*settingsObject)[setting->getName()].as<bool>())) {
          result.reason = String(setting->getName());
          result.reason.concat(F(" setting does not pass the validator function"));
          return result;
        }
      } else if (setting->isRequired()) {
        result.reason = String(setting->getName());
        result.reason.concat(F(" setting is missing"));
        return result;
      }
    } else if (iSetting->isLong()) {
      HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);

      if (settingsObject->containsKey(setting->getName())) {
        if (!(*settingsObject)[setting->getName()].is<long>()) {
          result.reason = String(setting->getName());
          result.reason.concat(F(" setting is not a long"));
          return result;
        } else if (!setting->validate((*settingsObject)[setting->getName()].as<long>())) {
          result.reason = String(setting->getName());
          result.reason.concat(F(" setting does not pass the validator function"));
          return result;
        }
      } else if (setting->isRequired()) {
        result.reason = String(setting->getName());
        result.reason.concat(F(" setting is missing"));
        return result;
      }
    } else if (iSetting->isDouble()) {
      HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);

      if (settingsObject->containsKey(setting->getName())) {
        if (!(*settingsObject)[setting->getName()].is<double>()) {
          result.reason = String(setting->getName());
          result.reason.concat(F(" setting is not a double"));
          return result;
        } else if (!setting->validate((*settingsObject)[setting->getName()].as<double>())) {
          result.reason = String(setting->getName());
          result.reason.concat((" setting does not pass the validator function"));
          return result;
        }
      } else if (setting->isRequired()) {
        result.reason = String(setting->getName());
        result.reason.concat(F(" setting is missing"));
        return result;
      }
    } else if (iSetting->isConstChar()) {
      HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);

      if (settingsObject->containsKey(setting->getName())) {
        if (!(*settingsObject)[setting->getName()].is<const char*>()) {
          result.reason = String(setting->getName());
          result.reason.concat(F(" setting is not a const char*"));
          return result;
        } else if (!setting->validate((*settingsObject)[setting->getName()].as<const char*>())) {
          result.reason = String(setting->getName());
          result.reason.concat(F(" setting does not pass the validator function"));
          return result;
        }
      } else if (setting->isRequired()) {
        result.reason = String(setting->getName());
        result.reason.concat(F(" setting is missing"));
        return result;
      }
    }
  }

  result.valid = true;
  return result;
}

#include "Validation.hpp"

using namespace HomieInternals;

ConfigValidationResult Validation::validateConfig(const JsonObject object) {
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

ConfigValidationResult Validation::_validateConfigRoot(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  const char* name = object["name"];
  if (!name) {
    result.reason = F("name is not a string");
    return result;
  }
  auto namelen = strlen(name);
  if (namelen == 0) {
    result.reason = F("name is empty");
    return result;
  }

  if (namelen + 1 > MAX_FRIENDLY_NAME_LENGTH) {
    result.reason = F("name is too long");
    return result;
  }

  if (object.containsKey("device_id")) {
    const char* device_id = object["device_id"];
    if (!device_id) {
      result.reason = F("device_id is not a string");
      return result;
    }
    if (strlen(device_id) + 1 > MAX_DEVICE_ID_LENGTH) {
      result.reason = F("device_id is too long");
      return result;
    }
  }

  if (object.containsKey(F("device_stats_interval")) && !object[F("device_stats_interval")].is<uint16_t>()) {
    result.reason = F("device_stats_interval is not an integer");
    return result;
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigWifi(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  JsonObject wifi = object["wifi"].as<JsonObject>();
  if (wifi.isNull()) {
    result.reason = F("wifi is not an object");
    return result;
  }

  {
    const char* wifiSsid = wifi["ssid"];
    if (!wifiSsid) {
      result.reason = F("wifi.ssid is not a string");
      return result;
    }
    const auto len = strlen(wifiSsid);
    if (len == 0) {
      result.reason = F("wifi.ssid is empty");
      return result;
    }
    if (len + 1 > MAX_WIFI_SSID_LENGTH) {
      result.reason = F("wifi.ssid is too long");
      return result;
    }
  }

  if (!wifi.containsKey("password") || !wifi["password"].is<const char*>()) {
    result.reason = F("wifi.password is not a string");
    return result;
  }
  if (wifi["password"] && strlen(wifi["password"]) + 1 > MAX_WIFI_PASSWORD_LENGTH) {
    result.reason = F("wifi.password is too long");
    return result;
  }

  if (wifi.containsKey("bssid") && !wifi["bssid"].is<const char*>()) {
    result.reason = F("wifi.bssid is not a string");
    return result;
  }
  if ((wifi.containsKey("bssid") && !wifi.containsKey("channel")) ||
    (!wifi.containsKey("bssid") && wifi.containsKey("channel"))) {
    result.reason = F("wifi.channel_bssid channel and BSSID is required");
    return result;
  }
  if (wifi.containsKey("bssid") && !Helpers::validateMacAddress(wifi.getMember("bssid").as<const char*>())) {
    result.reason = F("wifi.bssid is not valid mac");
    return result;
  }

  if (wifi.containsKey("channel") && !wifi["channel"].is<uint16_t>()) {
    result.reason = F("wifi.channel is not an integer");
    return result;
  }

  if (wifi.containsKey("ip") && !wifi["ip"].is<const char*>()) {
    result.reason = F("wifi.ip is not a string");
    return result;
  }
  if (wifi["ip"] && strlen(wifi["ip"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.ip is too long");
    return result;
  }
  if (wifi["ip"] && !Helpers::validateIP(wifi.getMember("ip").as<const char*>())) {
    result.reason = F("wifi.ip is not valid ip address");
    return result;
  }

  if (wifi.containsKey("mask") && !wifi["mask"].is<const char*>()) {
    result.reason = F("wifi.mask is not a string");
    return result;
  }
  if (wifi["mask"] && strlen(wifi["mask"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.mask is too long");
    return result;
  }
  if (wifi["mask"] && !Helpers::validateIP(wifi.getMember("mask").as<const char*>())) {
    result.reason = F("wifi.mask is not valid mask");
    return result;
  }

  if (wifi.containsKey("gw") && !wifi["gw"].is<const char*>()) {
    result.reason = F("wifi.gw is not a string");
    return result;
  }
  if (wifi["gw"] && strlen(wifi["gw"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.gw is too long");
    return result;
  }
  if (wifi["gw"] && !Helpers::validateIP(wifi.getMember("gw").as<const char*>())) {
    result.reason = F("wifi.gw is not valid gateway address");
    return result;
  }

  if (wifi.containsKey("ip") || wifi.containsKey("gw") || wifi.containsKey("mask")) {
    if (!(wifi.containsKey("ip") && wifi.containsKey("gw") && wifi.containsKey("mask"))) {
      result.reason = F("wifi.staticip ip, gw and mask is required");
      return result;
    }
  }

  if (wifi.containsKey("dns1") && !wifi["dns1"].is<const char*>()) {
    result.reason = F("wifi.dns1 is not a string");
    return result;
  }
  if (wifi["dns1"] && strlen(wifi["dns1"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.dns1 is too long");
    return result;
  }
  if (wifi["dns1"] && !Helpers::validateIP(wifi.getMember("dns1").as<const char*>())) {
    result.reason = F("wifi.dns1 is not valid dns address");
    return result;
  }

  if (wifi.containsKey("dns2") && !wifi.containsKey("dns1")) {
    result.reason = F("wifi.dns2 no dns1 defined");
    return result;
  }
  if (wifi.containsKey("dns2") && !wifi["dns2"].is<const char*>()) {
    result.reason = F("wifi.dns2 is not a string");
    return result;
  }
  if (wifi["dns2"] && strlen(wifi["dns2"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.dns2 is too long");
    return result;
  }
  if (wifi["dns2"] && !Helpers::validateIP(wifi.getMember("dns2").as<const char*>())) {
    result.reason = F("wifi.dns2 is not valid dns address");
    return result;
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigMqtt(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  if (!object.containsKey("mqtt") || !object["mqtt"].is<JsonObject>()) {
    result.reason = F("mqtt is not an object");
    return result;
  }
  if (!object["mqtt"].as<JsonObject>().containsKey("host") || !object["mqtt"]["host"].is<const char*>()) {
    result.reason = F("mqtt.host is not a string");
    return result;
  }
  if (strlen(object["mqtt"]["host"]) + 1 > MAX_HOSTNAME_LENGTH) {
    result.reason = F("mqtt.host is too long");
    return result;
  }
  if (object["mqtt"].as<JsonObject>().containsKey("port") && !object["mqtt"]["port"].is<uint16_t>()) {
    result.reason = F("mqtt.port is not an integer");
    return result;
  }
  if (object["mqtt"].as<JsonObject>().containsKey("base_topic")) {
    if (!object["mqtt"]["base_topic"].is<const char*>()) {
      result.reason = F("mqtt.base_topic is not a string");
      return result;
    }

    if (strlen(object["mqtt"]["base_topic"]) + 1 > MAX_MQTT_BASE_TOPIC_LENGTH) {
      result.reason = F("mqtt.base_topic is too long");
      return result;
    }
  }
  if (object["mqtt"].as<JsonObject>().containsKey("auth")) {
    if (!object["mqtt"]["auth"].is<bool>()) {
      result.reason = F("mqtt.auth is not a boolean");
      return result;
    }

    if (object["mqtt"]["auth"]) {
      if (!object["mqtt"].as<JsonObject>().containsKey("username") || !object["mqtt"]["username"].is<const char*>()) {
        result.reason = F("mqtt.username is not a string");
        return result;
      }
      if (strlen(object["mqtt"]["username"]) + 1 > MAX_MQTT_CREDS_LENGTH) {
        result.reason = F("mqtt.username is too long");
        return result;
      }
      if (!object["mqtt"].as<JsonObject>().containsKey("password") || !object["mqtt"]["password"].is<const char*>()) {
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

ConfigValidationResult Validation::_validateConfigOta(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  if (!object.containsKey("ota") || !object["ota"].is<JsonObject>()) {
    result.reason = F("ota is not an object");
    return result;
  }
  if (!object["ota"].as<JsonObject>().containsKey("enabled") || !object["ota"]["enabled"].is<bool>()) {
    result.reason = F("ota.enabled is not a boolean");
    return result;
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigSettings(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  StaticJsonDocument<0> emptySettingsDocument;

  JsonObject settingsObject = emptySettingsDocument.as<JsonObject>();

  if (object.containsKey("settings") && object["settings"].is<JsonObject>()) {
    settingsObject = object["settings"].as<JsonObject>();
  }

  if (settingsObject.size() > MAX_CONFIG_SETTING_SIZE) {//max settings here and in isettings
    result.reason = F("settings contains more elements than the set limit");
    return result;
  }

  for (IHomieSetting* iSetting : IHomieSetting::settings) {
    enum class Issue {
      Type,
      Validator,
      Missing
    };
    auto setReason = [&result, &iSetting](Issue issue) {
      switch (issue) {
      case Issue::Type:
        result.reason = String(iSetting->getName()) + F(" setting is not a ") + String(iSetting->getType());
        break;
      case Issue::Validator:
        result.reason = String(iSetting->getName()) + F(" setting does not pass the validator function");
        break;
      case Issue::Missing:
        result.reason = String(iSetting->getName()) + F(" setting is missing");
        break;
      }
    };

    if (iSetting->isBool()) {
      HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        if (!settingsObject[setting->getName()].is<bool>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingsObject[setting->getName()].as<bool>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isLong()) {
      HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        if (!settingsObject[setting->getName()].is<long>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingsObject[setting->getName()].as<long>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isDouble()) {
      HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        if (!settingsObject[setting->getName()].is<double>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingsObject[setting->getName()].as<double>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isConstChar()) {
      HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);

      if (settingsObject.containsKey(setting->getName())) {
        if (!settingsObject[setting->getName()].is<const char*>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingsObject[setting->getName()].as<const char*>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    }
  }

  result.valid = true;
  return result;
}

// bool Validation::_validateConfigWifiBssid(const char *mac) {
//   int i = 0;
//   int s = 0;
//   while (*mac) {
//    if (isxdigit(*mac)) {
//       i++;
//    }
//    else if (*mac == ':' || *mac == '-') {
//       if (i == 0 || i / 2 - 1 != s)
//         break;
//       ++s;
//    }
//    else {
//        s = -1;
//    }
//    ++mac;
//   }
//   return (i == MAX_MAC_STRING_LENGTH && s == 5);
// }

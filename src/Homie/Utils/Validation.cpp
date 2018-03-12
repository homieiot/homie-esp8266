#include "Validation.hpp"

using namespace HomieInternals;

ValidationResult Validation::validateConfig(const JsonObject& object) {
  JsonObject& objectPTR = (JsonObject&)object;
  _removeNullConfigItems("Config", &objectPTR);

  ValidationResult result;
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

void Validation::_removeNullConfigItems(const char* name, JsonObject* object) {
  auto hasValue = [](JsonVariant& val) {
    return val.is<bool>() || val.is<int>() || val.is<float>() || val.as<const char*>() != nullptr;
  };

  for (JsonPair kv : *object) {
    if (kv.value.is<JsonObject&>()) {
       _removeNullConfigItems(kv.key, &kv.value.as<JsonObject&>());
    } else if (!hasValue(kv.value)) {
       Interface::get().getLogger() << F("Removed ") << kv.key << F(" from ") << name << endl;
       object->remove(kv.key);
    }
  }
}


ValidationResult Validation::_validateConfigRoot(const JsonObject& object) {
  ValidationResult result;
  result.valid = false;
  if (!object.containsKey("name") || !object["name"].is<const char*>()) {
    result.reason = F("name is not a string");
    return result;
  }
  if (strlen(object["name"]) + 1 > MAX_FRIENDLY_NAME_STRING_LENGTH) {
    result.reason = F("name is too long");
    return result;
  }
  if (object.containsKey("device_id")) {
    if (!object["device_id"].is<const char*>()) {
      result.reason = F("device_id is not a string");
      return result;
    }
    if (strlen(object["device_id"]) + 1 > MAX_DEVICE_ID_STRING_LENGTH) {
      result.reason = F("device_id is too long");
      return result;
    }
  }

  const char* name = object["name"];

  if (strcmp_P(name, PSTR("")) == 0) {
    result.reason = F("name is empty");
    return result;
  }

  if (object.containsKey(F("device_stats_interval")) && !object[F("device_stats_interval")].is<uint16_t>()) {
    result.reason = F("device_stats_interval is not an integer");
    return result;
  }

  result.valid = true;
  return result;
}

ValidationResult Validation::_validateConfigWifi(const JsonObject& object) {
  ValidationResult result;
  result.valid = false;

  if (!object.containsKey("wifi") || !object["wifi"].is<JsonObject>()) {
    result.reason = F("wifi is not an object");
    return result;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("ssid") || !object["wifi"]["ssid"].is<const char*>()) {
    result.reason = F("wifi.ssid is not a string");
    return result;
  }
  if (strlen(object["wifi"]["ssid"]) + 1 > MAX_WIFI_SSID_STRING_LENGTH) {
    result.reason = F("wifi.ssid is too long");
    return result;
  }
  if (!object["wifi"].as<JsonObject&>().containsKey("password") || !object["wifi"]["password"].is<const char*>()) {
    result.reason = F("wifi.password is not a string");
    return result;
  }
  if (object["wifi"]["password"] && strlen(object["wifi"]["password"]) + 1 > MAX_WIFI_PASSWORD_STRING_LENGTH) {
    result.reason = F("wifi.password is too long");
    return result;
  }
  // by benzino
  if (object["wifi"].as<JsonObject&>().containsKey("bssid") && !object["wifi"]["bssid"].is<const char*>()) {
    result.reason = F("wifi.bssid is not a string");
    return result;
  }
  if ((object["wifi"].as<JsonObject&>().containsKey("bssid") && !object["wifi"].as<JsonObject&>().containsKey("channel")) ||
    (!object["wifi"].as<JsonObject&>().containsKey("bssid") && object["wifi"].as<JsonObject&>().containsKey("channel"))) {
    result.reason = F("wifi.channel_bssid channel and BSSID is required");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("bssid") && !Helpers::validateMacAddress(object["wifi"].as<JsonObject&>().get<const char*>("bssid"))) {
    result.reason = F("wifi.bssid is not valid mac");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("channel") && !object["wifi"]["channel"].is<uint16_t>()) {
    result.reason = F("wifi.channel is not an integer");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("ip") && !object["wifi"]["ip"].is<const char*>()) {
    result.reason = F("wifi.ip is not a string");
    return result;
  }
  if (object["wifi"]["ip"] && strlen(object["wifi"]["ip"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.ip is too long");
    return result;
  }
  if (object["wifi"]["ip"] && !Helpers::validateIP(object["wifi"].as<JsonObject&>().get<const char*>("ip"))) {
    result.reason = F("wifi.ip is not valid ip address");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("mask") && !object["wifi"]["mask"].is<const char*>()) {
    result.reason = F("wifi.mask is not a string");
    return result;
  }
  if (object["wifi"]["mask"] && strlen(object["wifi"]["mask"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.mask is too long");
    return result;
  }
  if (object["wifi"]["mask"] && !Helpers::validateIP(object["wifi"].as<JsonObject&>().get<const char*>("mask"))) {
    result.reason = F("wifi.mask is not valid mask");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("gw") && !object["wifi"]["gw"].is<const char*>()) {
    result.reason = F("wifi.gw is not a string");
    return result;
  }
  if (object["wifi"]["gw"] && strlen(object["wifi"]["gw"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.gw is too long");
    return result;
  }
  if (object["wifi"]["gw"] && !Helpers::validateIP(object["wifi"].as<JsonObject&>().get<const char*>("gw"))) {
    result.reason = F("wifi.gw is not valid gateway address");
    return result;
  }
  if ((object["wifi"].as<JsonObject&>().containsKey("ip") && (!object["wifi"].as<JsonObject&>().containsKey("mask") || !object["wifi"].as<JsonObject&>().containsKey("gw"))) ||
    (object["wifi"].as<JsonObject&>().containsKey("gw") && (!object["wifi"].as<JsonObject&>().containsKey("mask") || !object["wifi"].as<JsonObject&>().containsKey("ip"))) ||
    (object["wifi"].as<JsonObject&>().containsKey("mask") && (!object["wifi"].as<JsonObject&>().containsKey("ip") || !object["wifi"].as<JsonObject&>().containsKey("gw")))) {
    result.reason = F("wifi.staticip ip, gw and mask is required");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("dns1") && !object["wifi"]["dns1"].is<const char*>()) {
    result.reason = F("wifi.dns1 is not a string");
    return result;
  }
  if (object["wifi"]["dns1"] && strlen(object["wifi"]["dns1"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.dns1 is too long");
    return result;
  }
  if (object["wifi"]["dns1"] && !Helpers::validateIP(object["wifi"].as<JsonObject&>().get<const char*>("dns1"))) {
    result.reason = F("wifi.dns1 is not valid dns address");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("dns2") && !object["wifi"].as<JsonObject&>().containsKey("dns1")) {
    result.reason = F("wifi.dns2 no dns1 defined");
    return result;
  }
  if (object["wifi"].as<JsonObject&>().containsKey("dns2") && !object["wifi"]["dns2"].is<const char*>()) {
    result.reason = F("wifi.dns2 is not a string");
    return result;
  }
  if (object["wifi"]["dns2"] && strlen(object["wifi"]["dns2"]) + 1 > MAX_IP_STRING_LENGTH) {
    result.reason = F("wifi.dns2 is too long");
    return result;
  }
  if (object["wifi"]["dns2"] && !Helpers::validateIP(object["wifi"].as<JsonObject&>().get<const char*>("dns2"))) {
    result.reason = F("wifi.dns2 is not valid dns address");
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

ValidationResult Validation::_validateConfigMqtt(const JsonObject& object) {
  ValidationResult result;
  result.valid = false;

  if (!object.containsKey("mqtt") || !object["mqtt"].is<JsonObject>()) {
    result.reason = F("mqtt is not an object");
    return result;
  }
  if (!object["mqtt"].as<JsonObject&>().containsKey("host") || !object["mqtt"]["host"].is<const char*>()) {
    result.reason = F("mqtt.host is not a string");
    return result;
  }
  if (strlen(object["mqtt"]["host"]) + 1 > MAX_HOSTNAME_STRING_LENGTH) {
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

    if (strlen(object["mqtt"]["base_topic"]) + 1 > MAX_MQTT_BASE_TOPIC_STRING_LENGTH) {
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
      if (strlen(object["mqtt"]["username"]) + 1 > MAX_MQTT_CREDS_STRING_LENGTH) {
        result.reason = F("mqtt.username is too long");
        return result;
      }
      if (!object["mqtt"].as<JsonObject&>().containsKey("password") || !object["mqtt"]["password"].is<const char*>()) {
        result.reason = F("mqtt.password is not a string");
        return result;
      }
      if (strlen(object["mqtt"]["password"]) + 1 > MAX_MQTT_CREDS_STRING_LENGTH) {
        result.reason = F("mqtt.password is too long");
        return result;
      }
    }
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("ssl") && !object["mqtt"]["ssl"].is<bool>()) {
    result.reason = F("mqtt.ssl is not a bool");
    return result;
  }
  if (object["mqtt"].as<JsonObject&>().containsKey("ssl_fingerprint")) {
    if (!object["mqtt"]["ssl_fingerprint"].is<const char*>()) {
      result.reason = F("mqtt.ssl_fingerprint is not a string");
      return result;
    }

    if (strlen(object["mqtt"]["ssl_fingerprint"]) + 1 != MAX_FINGERPRINT_STRING_LENGTH) {
      result.reason = F("mqtt.ssl_fingerprint is not the right length");
      return result;
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

ValidationResult Validation::_validateConfigOta(const JsonObject& object) {
  ValidationResult result;
  result.valid = false;

  if (!object.containsKey("ota") || !object["ota"].is<JsonObject>()) {
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

ValidationResult Validation::_validateConfigSettings(const JsonObject& object) {
  ValidationResult result;
  result.valid = false;

  if (!object.containsKey("settings")) {
    result.valid = true;
    return result;
  }

  if (!object["settings"].is<JsonObject>()) {
    result.reason = F("settings is not an object");
    return result;
  }

  const JsonObject& settingsObject = object["settings"].as<JsonObject&>();

  if (settingsObject.size() > MAX_CONFIG_SETTING_SIZE) {
    result.reason = F("settings contains more elements than the set limit");
    return result;
  }

  enum class Issue {
    None,
    Type,
    Validator,
    Required
  };

  for (IHomieSetting& iSetting : IHomieSetting::settings) {
    Issue issue = Issue::None;

    if (iSetting.isBool()) {
      HomieSetting<bool>& setting = static_cast<HomieSetting<bool>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        if (!settingsObject[setting.getName()].is<bool>()) {
          issue = Issue::Type;
        } else if (!setting._validate(settingsObject[setting.getName()].as<bool>())) {
          issue = Issue::Validator;
        }
      } else if (setting.isRequired()) {
        issue = Issue::Required;
      }
    } else if (iSetting.isLong()) {
      HomieSetting<long>& setting = static_cast<HomieSetting<long>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        if (!settingsObject[setting.getName()].is<long>()) {
          issue = Issue::Type;
        } else if (!setting._validate(settingsObject[setting.getName()].as<long>())) {
          issue = Issue::Validator;
        }
      } else if (setting.isRequired()) {
        issue = Issue::Required;
      }
    } else if (iSetting.isDouble()) {
      HomieSetting<double>& setting = static_cast<HomieSetting<double>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        if (!settingsObject[setting.getName()].is<double>()) {
          issue = Issue::Type;
        } else if (!setting._validate(settingsObject[setting.getName()].as<double>())) {
          issue = Issue::Validator;
        }
      } else if (setting.isRequired()) {
        issue = Issue::Required;
      }
    } else if (iSetting.isConstChar()) {
      HomieSetting<const char*>& setting = static_cast<HomieSetting<const char*>&>(iSetting);

      if (settingsObject.containsKey(setting.getName())) {
        if (!settingsObject[setting.getName()].is<const char*>()) {
          issue = Issue::Type;

        } else if (!setting._validate(settingsObject[setting.getName()].as<const char*>())) {
          issue = Issue::Validator;
        }
      } else if (setting.isRequired()) {
        issue = Issue::Required;
      }
    }
    if (issue != Issue::None) {
      switch (issue) {
        case Issue::Type:
          result.reason = String(iSetting.getName()) + F(" setting is not a ") + String(iSetting.getType());
          break;
        case Issue::Validator:
          result.reason = String(iSetting.getName()) + F(" setting does not pass the validator function");
          break;
        case Issue::Required:
          result.reason = String(iSetting.getName()) + F(" setting is missing. Required!");
          break;
      }
      return result;
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

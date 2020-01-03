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

  {
    JsonVariant name = object["name"];

    if (!name.as<const char*>()) {
      result.reason = F("name is not a string");
      return result;
    }
    if (strlen(name.as<const char*>()) + 1 > MAX_FRIENDLY_NAME_LENGTH) {
      result.reason = F("name is too long");
      return result;
    }

    if (strcmp_P(name.as<const char*>(), PSTR("")) == 0) {
      result.reason = F("name is empty");
      return result;
    }
  }

  {
    JsonVariant deviceId = object["device_id"];

    if (!deviceId.isNull()) {
      if (!deviceId.as<const char*>()) {
        result.reason = F("device_id is not a string");
        return result;
      }
      if (strlen(deviceId.as<const char*>()) + 1 > MAX_DEVICE_ID_LENGTH) {
        result.reason = F("device_id is too long");
        return result;
      }
    }
  }

  {
    JsonVariant deviceStatsInterval = object["device_stats_interval"];

    if (!deviceStatsInterval.isNull()) {
      if (!deviceStatsInterval.is<uint16_t>()) {
        result.reason = F("device_stats_interval is not an integer");
        return result;
      }
    }
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigWifi(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  JsonVariant wifi = object["wifi"];

  if (!wifi.is<JsonObject>()) {
    result.reason = F("wifi is not an object");
    return result;
  }

  {
    JsonVariant wifiSsid = wifi["ssid"];

    if (!wifiSsid.as<const char*>()) {
      result.reason = F("wifi.ssid is not a string");
      return result;
    }
    if (strlen(wifiSsid.as<const char*>()) + 1 > MAX_WIFI_SSID_LENGTH) {
      result.reason = F("wifi.ssid is too long");
      return result;
    }
    if (strcmp_P(wifiSsid.as<const char*>(), PSTR("")) == 0) {
      result.reason = F("wifi.ssid is empty");
      return result;
    }
  }

  {
    JsonVariant wifiPassword = wifi["password"];

    if (!wifiPassword.isNull()) {
      if (!wifiPassword.as<const char*>()) {
        result.reason = F("wifi.password is not a string");
        return result;
      }
      if (strlen(wifiPassword.as<const char*>()) + 1 > MAX_WIFI_PASSWORD_LENGTH) {
        result.reason = F("wifi.password is too long");
        return result;
      }
    }
  }

  {
    JsonVariant wifiBssid = wifi["bssid"];
    JsonVariant wifiChannel = wifi["channel"];

    if ((wifiBssid.isNull() || wifiChannel.isNull()) != (wifiBssid.isNull() && wifiChannel.isNull())) {
      result.reason = F("wifi.channel_bssid channel and BSSID is required");
      return result;
    }
    if (!wifiBssid.isNull()) {
      if (!wifiBssid.as<const char*>()) {
        result.reason = F("wifi.bssid is not a string");
        return result;
      }
      if (!Helpers::validateMacAddress(wifiBssid.as<const char*>())) {
        result.reason = F("wifi.bssid is not valid mac");
        return result;
      }
      if (!wifiChannel.is<uint16_t>()) {
        result.reason = F("wifi.channel is not an integer");
        return result;
      }
    }
  }

  {
    JsonVariant wifiIp = wifi["ip"];
    JsonVariant wifiMask = wifi["mask"];
    JsonVariant wifiGw = wifi["gw"];

    if ((wifiIp.isNull() || wifiMask.isNull() || wifiGw.isNull()) != (wifiIp.isNull() && wifiMask.isNull() && wifiGw.isNull())) {
      result.reason = F("wifi.staticip ip, gw and mask is required");
      return result;
    }
    if (!wifiIp.isNull()) {
      if (!wifiIp.as<const char*>()) {
        result.reason = F("wifi.ip is not a string");
        return result;
      }
      if (strlen(wifiIp.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.ip is too long");
        return result;
      }
      if (!Helpers::validateIP(wifiIp.as<const char*>())) {
        result.reason = F("wifi.ip is not valid ip address");
        return result;
      }
      if (!wifiMask.as<const char*>()) {
        result.reason = F("wifi.mask is not a string");
        return result;
      }
      if (strlen(wifiMask.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.mask is too long");
        return result;
      }
      if (!Helpers::validateIP(wifiMask.as<const char*>())) {
        result.reason = F("wifi.mask is not valid mask");
        return result;
      }
      if (!wifiGw.as<const char*>()) {
        result.reason = F("wifi.gw is not a string");
        return result;
      }
      if (strlen(wifiGw.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.gw is too long");
        return result;
      }
      if (!Helpers::validateIP(wifiGw.as<const char*>())) {
        result.reason = F("wifi.gw is not valid gateway address");
        return result;
      }
    }
  }

  {
    JsonVariant wifiDns1 = wifi["dns1"];
    JsonVariant wifiDns2 = wifi["dns2"];

    if (!wifiDns1.isNull()) {
      if (!wifiDns1.as<const char*>()) {
        result.reason = F("wifi.dns1 is not a string");
        return result;
      }
      if (strlen(wifiDns1.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.dns1 is too long");
        return result;
      }
      if (!Helpers::validateIP(wifiDns1.as<const char*>())) {
        result.reason = F("wifi.dns1 is not valid dns address");
        return result;
      }
    }
    if (!wifiDns2.isNull()) {
      if (!wifiDns2.as<const char*>()) {
        result.reason = F("wifi.dns2 is not a string");
        return result;
      }
      if (strlen(wifiDns2.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.dns2 is too long");
        return result;
      }
      if (!Helpers::validateIP(wifiDns2.as<const char*>())) {
        result.reason = F("wifi.dns2 is not valid dns address");
        return result;
      }
      if (wifiDns1.isNull()) {
        result.reason = F("wifi.dns2 no dns1 defined");
        return result;
      }
    }
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigMqtt(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  JsonVariant mqtt = object["mqtt"];

  if (!mqtt.is<JsonObject>()) {
    result.reason = F("mqtt is not an object");
    return result;
  }

  {
    JsonVariant mqttHost = mqtt["host"];

    if (!mqttHost.as<const char*>()) {
      result.reason = F("mqtt.host is not a string");
      return result;
    }
    if (strlen(mqttHost.as<const char*>()) + 1 > MAX_HOSTNAME_LENGTH) {
      result.reason = F("mqtt.host is too long");
      return result;
    }
    if (strcmp_P(mqttHost.as<const char*>(), PSTR("")) == 0) {
      result.reason = F("mqtt.host is empty");
      return result;
    }
  }

  {
    JsonVariant mqttPort = mqtt["port"];

    if (!mqttPort.isNull() && !mqttPort.is<uint16_t>()) {
      result.reason = F("mqtt.port is not an integer");
      return result;
    }
  }

  {
    JsonVariant mqttSsl = mqtt["ssl"];
    if (!mqttSsl.isNull() && !mqttSsl.is<bool>()) {
      result.reason = F("mqtt.ssl is not a bool");
      return result;
    }
  }

  {
    JsonVariant mqttSslFingerprint = mqtt["ssl_fingerprint"];
    if (!mqttSslFingerprint.isNull()) {
      if (!mqttSslFingerprint.as<const char*>()) {
        result.reason = F("mqtt.ssl_fingerprint is not a string");
        return result;
      }
      if (strlen(mqttSslFingerprint.as<const char*>()) > MAX_FINGERPRINT_SIZE * 2) {
        result.reason = F("mqtt.ssl_fingerprint is too long");
        return result;
      }
    }
  }

  {
    JsonVariant mqttBaseTopic = mqtt["base_topic"];
    if (!mqttBaseTopic.isNull()) {
      if (!mqttBaseTopic.as<const char*>()) {
        result.reason = F("mqtt.base_topic is not a string");
        return result;
      }
      if (strlen(mqttBaseTopic.as<const char*>()) + 1 > MAX_MQTT_BASE_TOPIC_LENGTH) {
        result.reason = F("mqtt.base_topic is too long");
        return result;
      }
    }
  }

  {
    JsonVariant mqttAuth = mqtt["auth"];

    if (!mqttAuth.isNull()) {
      if (!mqttAuth.is<bool>()) {
        result.reason = F("mqtt.auth is not a boolean");
        return result;
      }

      if (mqttAuth.as<bool>()) {
        JsonVariant mqttUsername = mqtt["username"];
        JsonVariant mqttPassword = mqtt["password"];

        if (!mqttUsername.as<const char*>()) {
          result.reason = F("mqtt.username is not a string");
          return result;
        }
        if (strlen(mqttUsername.as<const char*>()) + 1 > MAX_MQTT_CREDS_LENGTH) {
          result.reason = F("mqtt.username is too long");
          return result;
        }
        if (!mqttPassword.as<const char*>()) {
          result.reason = F("mqtt.password is not a string");
          return result;
        }
        if (strlen(mqttPassword.as<const char*>()) + 1 > MAX_MQTT_CREDS_LENGTH) {
          result.reason = F("mqtt.password is too long");
          return result;
        }
      }
    }
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigOta(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  JsonVariant ota = object["ota"];

  if (!ota.isNull()) {
    if (!ota.is<JsonObject>()) {
      result.reason = F("ota is not an object");
      return result;
    }

    {
      JsonVariant otaEnabled = ota["enabled"];

      if (!otaEnabled.isNull() && !otaEnabled.is<bool>()) {
        result.reason = F("ota.enabled is not a boolean");
        return result;
      }
    }
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigSettings(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  StaticJsonDocument<0> emptySettingsDoc;

  JsonObject settingsObject = object["settings"] | emptySettingsDoc.to<JsonObject>();

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

    JsonVariant settingElement = settingsObject[iSetting->getName()];

    if (iSetting->isBool()) {
      HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);

      if (!settingElement.isNull()) {
        if (!settingElement.is<bool>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingElement.as<bool>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isLong()) {
      HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);

      if (!settingElement.isNull()) {
        if (!settingElement.is<long>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingElement.as<long>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isDouble()) {
      HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);

      if (!settingElement.isNull()) {
        if (!settingElement.is<double>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingElement.as<double>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isConstChar()) {
      HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);

      if (!settingElement.isNull()) {
        if (!settingElement.is<const char*>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(settingElement.as<const char*>())) {
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
//   return (i == MAX_MAC_LENGTH * 2 && s == 5);
// }

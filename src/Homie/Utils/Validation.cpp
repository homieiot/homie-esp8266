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
    JsonVariant jvName = object["name"];

    if (!jvName.is<const char*>()) {
      result.reason = F("name is not a string");
      return result;
    }
    if (!jvName.as<const char*>()) {
      result.reason = F("name is null");
      return result;
    }
    if (strlen(jvName.as<const char*>()) + 1 > MAX_FRIENDLY_NAME_LENGTH) {
      result.reason = F("name is too long");
      return result;
    }
    if (strcmp_P(jvName.as<const char*>(), PSTR("")) == 0) {
      result.reason = F("name is empty");
      return result;
    }
  }

  {
    JsonVariant jvDeviceId = object["device_id"];

    if (!jvDeviceId.isNull()) {
      if (!jvDeviceId.is<const char*>()) {
        result.reason = F("device_id is not a string");
        return result;
      }
      if (!jvDeviceId.as<const char*>()) {
        result.reason = F("device_id is null");
        return result;
      }
      if (strlen(jvDeviceId.as<const char*>()) + 1 > MAX_DEVICE_ID_LENGTH) {
        result.reason = F("device_id is too long");
        return result;
      }
    }
  }

  {
    JsonVariant jvDeviceStatsInterval = object["device_stats_interval"];

    if (!jvDeviceStatsInterval.isNull() && !jvDeviceStatsInterval.is<uint16_t>()) {
      result.reason = F("device_stats_interval is not an integer");
      return result;
    }
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigWifi(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  JsonVariant jvWiFi = object["wifi"];

  if (!jvWiFi.is<JsonObject>()) {
    result.reason = F("wifi is not an object");
    return result;
  }

  {
    JsonVariant jvWiFiSsid = jvWiFi["ssid"];

    if (!jvWiFiSsid.is<const char*>()) {
      result.reason = F("wifi.ssid is not a string");
      return result;
    }
    if (!jvWiFiSsid.as<const char*>()) {
      result.reason = F("wifi.ssid is null");
      return result;
    }
    if (strlen(jvWiFiSsid.as<const char*>()) + 1 > MAX_WIFI_SSID_LENGTH) {
      result.reason = F("wifi.ssid is too long");
      return result;
    }
    if (strcmp_P(jvWiFiSsid.as<const char*>(), PSTR("")) == 0) {
      result.reason = F("wifi.ssid is empty");
      return result;
    }
  }

  {
    JsonVariant jvWiFiPassword = jvWiFi["password"];

    if (!jvWiFiPassword.is<const char*>()) {
      result.reason = F("wifi.password is not a string");
      return result;
    }
    if (jvWiFiPassword.as<const char*>() && strlen(jvWiFiPassword.as<const char*>()) + 1 > MAX_WIFI_PASSWORD_LENGTH) {
      result.reason = F("wifi.password is too long");
      return result;
    }
  }

  {
    JsonVariant jvWiFiBssid = jvWiFi["bssid"];
    JsonVariant jvWiFiChannel = jvWiFi["channel"];

    if (!jvWiFiBssid.isNull()) {
      if (!jvWiFiBssid.is<const char*>()) {
        result.reason = F("wifi.bssid is not a string");
        return result;
      }
      if (!jvWiFiBssid.as<const char*>()) {
        result.reason = F("wifi.bssid is null");
        return result;
      }
      if (!Helpers::validateMacAddress(jvWiFiBssid.as<const char*>())) {
        result.reason = F("wifi.bssid is not valid mac");
        return result;
      }
    }
    if (!jvWiFiChannel.isNull() && !jvWiFiChannel.is<uint16_t>()) {
      result.reason = F("wifi.channel is not an integer");
      return result;
    }
    if ((!jvWiFiBssid.isNull() && jvWiFiChannel.isNull()) || (jvWiFiBssid.isNull() && !jvWiFiChannel.isNull())) {
      result.reason = F("wifi.channel_bssid channel and BSSID is required");
      return result;
    }
  }

  {
    JsonVariant jvWiFiIp = jvWiFi["ip"];
    JsonVariant jvWiFiMask = jvWiFi["mask"];
    JsonVariant jvWiFiGateway = jvWiFi["gw"];

    if (!jvWiFiIp.isNull()) {
      if (!jvWiFiIp.is<const char*>()) {
        result.reason = F("wifi.ip is not a string");
        return result;
      }
      if (!jvWiFiIp.as<const char*>()) {
        result.reason = F("wifi.ip is null");
        return result;
      }
      if (strlen(jvWiFiIp.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.ip is too long");
        return result;
      }
      if (!Helpers::validateIP(jvWiFiIp.as<const char*>())) {
        result.reason = F("wifi.ip is not valid ip address");
        return result;
      }
    }

    if (!jvWiFiMask.isNull()) {
      if (!jvWiFiMask.is<const char*>()) {
        result.reason = F("wifi.mask is not a string");
        return result;
      }
      if (!jvWiFiMask.as<const char*>()) {
        result.reason = F("wifi.mask is null");
        return result;
      }
      if (strlen(jvWiFiMask.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.mask is too long");
        return result;
      }
      if (!Helpers::validateIP(jvWiFiMask.as<const char*>())) {
        result.reason = F("wifi.mask is not a valid mask");
        return result;
      }
    }

    if (!jvWiFiGateway.isNull()) {
      if (!jvWiFiGateway.is<const char*>()) {
        result.reason = F("wifi.gw is not a string");
        return result;
      }
      if (!jvWiFiGateway.as<const char*>()) {
        result.reason = F("wifi.gw is null");
        return result;
      }
      if (strlen(jvWiFiGateway.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.gw is too long");
        return result;
      }
      if (!Helpers::validateIP(jvWiFiGateway.as<const char*>())) {
        result.reason = F("wifi.gw is not valid gateway address");
        return result;
      }
    }

    if ((!jvWiFiIp.isNull() && (jvWiFiMask.isNull() || jvWiFiGateway.isNull())) ||
      (!jvWiFiGateway.isNull() && (jvWiFiMask.isNull() || jvWiFiIp.isNull())) ||
      (!jvWiFiMask.isNull() && (jvWiFiIp.isNull() || jvWiFiGateway.isNull()))) {
      result.reason = F("wifi.staticip ip, gw and mask is required");
      return result;
    }
  }

{
    JsonVariant jvWiFiDns1 = jvWiFi["dns1"];
    JsonVariant jvWiFiDns2 = jvWiFi["dns2"];

    if (!jvWiFiDns1.isNull()) {
      if (!jvWiFiDns1.is<const char*>()) {
        result.reason = F("wifi.dns1 is not a string");
        return result;
      }
      if (!jvWiFiDns1.as<const char*>()) {
        result.reason = F("wifi.dns1 is null");
        return result;
      }
      if (strlen(jvWiFiDns1.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.dns1 is too long");
        return result;
      }
      if (!Helpers::validateIP(jvWiFiDns1.as<const char*>())) {
        result.reason = F("wifi.dns1 is not a valid dns address");
        return result;
      }
    }

    if (!jvWiFiDns2.isNull()) {
      if (jvWiFiDns1.isNull()) {
        result.reason = F("wifi.dns2 no dns1 defined");
        return result;
      }
      if (!jvWiFiDns2.is<const char*>()) {
        result.reason = F("wifi.dns2 is not a string");
        return result;
      }
      if (!jvWiFiDns2.as<const char*>()) {
        result.reason = F("wifi.dns2 is null");
        return result;
      }
      if (strlen(jvWiFiDns2.as<const char*>()) + 1 > MAX_IP_STRING_LENGTH) {
        result.reason = F("wifi.dns2 is too long");
        return result;
      }
      if (!Helpers::validateIP(jvWiFiDns2.as<const char*>())) {
        result.reason = F("wifi.dns2 is not a valid dns address");
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

  JsonVariant jvMqtt = object["mqtt"];

  if (!jvMqtt.is<JsonObject>()) {
    result.reason = F("mqtt is not an object");
    return result;
  }

  {
    JsonVariant jvMqttHost = jvMqtt["host"];

    if (!jvMqttHost.is<const char*>()) {
      result.reason = F("mqtt.host is not a string");
      return result;
    }
    if (!jvMqttHost.as<const char*>()) {
      result.reason = F("mqtt.host is null");
      return result;
    }
    if (strlen(jvMqttHost.as<const char*>()) + 1 > MAX_HOSTNAME_LENGTH) {
      result.reason = F("mqtt.host is too long");
      return result;
    }
    if (strcmp_P(jvMqttHost.as<const char*>(), PSTR("")) == 0) {
      result.reason = F("mqtt.host is empty");
      return result;
    }
  }

  {
    JsonVariant jvMqttPort = jvMqtt["port"];

    if (!jvMqttPort.isNull() && !jvMqttPort.is<uint16_t>()) {
      result.reason = F("mqtt.port is not an integer");
      return result;
    }
  }

  {
    JsonVariant jvMqttSsl = jvMqtt["ssl"];

    if (!jvMqttSsl.isNull() && !jvMqttSsl.is<bool>()) {
      result.reason = F("mqtt.ssl is not a bool");
      return result;
    }
  }

  {
    JsonVariant jvMqttSslFingerprint = jvMqtt["ssl_fingerprint"];

    if (!jvMqttSslFingerprint.isNull()) {
      if (!jvMqttSslFingerprint.is<const char*>()) {
        result.reason = F("mqtt.ssl_fingerprint is not a string");
        return result;
      }
      if (!jvMqttSslFingerprint.as<const char*>()) {
        result.reason = F("mqtt.ssl_fingerprint is null");
        return result;
      }
      if (strlen(jvMqttSslFingerprint.as<const char*>()) > MAX_FINGERPRINT_SIZE * 2) {
        result.reason = F("mqtt.ssl_fingerprint is too long");
        return result;
      }
    }
  }

  {
    JsonVariant jvMqttBaseTopic = jvMqtt["base_topic"];

    if (!jvMqttBaseTopic.isNull()) {
      if (!jvMqttBaseTopic.is<const char*>()) {
        result.reason = F("mqtt.base_topic is not a string");
        return result;
      }
      if (!jvMqttBaseTopic.as<const char*>()) {
        result.reason = F("mqtt.base_topic is null");
        return result;
      }
      if (strlen(jvMqttBaseTopic.as<const char*>()) + 1 > MAX_MQTT_BASE_TOPIC_LENGTH) {
        result.reason = F("mqtt.base_topic is too long");
        return result;
      }
    }
  }

  {
    JsonVariant jvMqttAuth = jvMqtt["auth"];

    if (!jvMqttAuth.isNull()) {
      if (!jvMqttAuth.is<bool>()) {
        result.reason = F("mqtt.auth is not a boolean");
        return result;
      }
      if (jvMqttAuth.as<bool>()) {
        JsonVariant jvMqttUsername = jvMqtt["username"];
        JsonVariant jvMqttPassword = jvMqtt["password"];

        if (!jvMqttUsername.is<const char*>()) {
          result.reason = F("mqtt.username is not a string");
          return result;
        }
        if (!jvMqttUsername.as<const char*>()) {
          result.reason = F("mqtt.username is null");
          return result;
        }
        if (strlen(jvMqttUsername.as<const char*>()) + 1 > MAX_MQTT_CREDS_LENGTH) {
          result.reason = F("mqtt.username is too long");
          return result;
        }
        if (!jvMqttPassword.is<const char*>()) {
          result.reason = F("mqtt.password is not a string");
          return result;
        }
        if (!jvMqttPassword.as<const char*>()) {
          result.reason = F("mqtt.password is null");
          return result;
        }
        if (strlen(jvMqttPassword.as<const char*>()) + 1 > MAX_MQTT_CREDS_LENGTH) {
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

  JsonVariant jvOta = object["ota"];

  if (!jvOta.is<JsonObject>()) {
    result.reason = F("ota is not an object");
    return result;
  }

  {
    JsonVariant jvOtaEnabled = jvOta["enabled"];

    if (!jvOtaEnabled.is<bool>()) {
      result.reason = F("ota.enabled is not a boolean");
      return result;
    }
  }

  result.valid = true;
  return result;
}

ConfigValidationResult Validation::_validateConfigSettings(const JsonObject object) {
  ConfigValidationResult result;
  result.valid = false;

  StaticJsonDocument<0> emptySettingsDoc;

  JsonObject settingsObject = emptySettingsDoc.to<JsonObject>();

  JsonVariant jvSettings = object["settings"];

  if (!jvSettings.isNull()) {
    if (jvSettings.is<JsonObject>()) {
      settingsObject = jvSettings.as<JsonObject>();
    } else {
      result.reason = F("settings is not an object");
      return result;
    }
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

    JsonVariant jvSetting = settingsObject[iSetting->getName()];

    if (iSetting->isBool()) {
      HomieSetting<bool>* setting = static_cast<HomieSetting<bool>*>(iSetting);

      if (!jvSetting.isNull()) {
        if (!jvSetting.is<bool>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(jvSetting.as<bool>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isLong()) {
      HomieSetting<long>* setting = static_cast<HomieSetting<long>*>(iSetting);

      if (!jvSetting.isNull()) {
        if (!jvSetting.is<long>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(jvSetting.as<long>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isDouble()) {
      HomieSetting<double>* setting = static_cast<HomieSetting<double>*>(iSetting);

      if (!jvSetting.isNull()) {
        if (!jvSetting.is<double>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(jvSetting.as<double>())) {
          setReason(Issue::Validator);
          return result;
        }
      } else if (setting->isRequired()) {
        setReason(Issue::Missing);
        return result;
      }
    } else if (iSetting->isConstChar()) {
      HomieSetting<const char*>* setting = static_cast<HomieSetting<const char*>*>(iSetting);

      if (!jvSetting.isNull()) {
        if (!jvSetting.is<const char*>()) {
          setReason(Issue::Type);
          return result;
        } else if (!setting->validate(jvSetting.as<const char*>())) {
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

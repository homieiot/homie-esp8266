#include "Config.hpp"

using namespace HomieInternals;

ConfigClass::ConfigClass()
: _eeprom_began(false)
, _custom_eeprom_size(0)
{
}

void ConfigClass::_eepromBegin() {
  if (!this->_eeprom_began) {
    EEPROM.begin(EEPROM_CONFIG_SIZE + this->_custom_eeprom_size);
    this->_eeprom_began = true;
  }
}

bool ConfigClass::load() {
  this->_eepromBegin();

  EEPROM.get(EEPROM_OFFSET, this->_config_struct);

  if (!this->_config_struct.configured) {
    return false;
  }

  if (this->_config_struct.boot_mode != BOOT_CONFIG && this->_config_struct.boot_mode != BOOT_NORMAL && this->_config_struct.boot_mode != BOOT_OTA) {
    return false;
  }

  return true;
}

ConfigStruct& ConfigClass::get() {
  return this->_config_struct;
}

void ConfigClass::save() {
  this->_eepromBegin();

  EEPROM.put(EEPROM_OFFSET, this->_config_struct);
  EEPROM.commit();
}

void ConfigClass::setCustomEepromSize(int count) {
  this->_custom_eeprom_size = count;
}

void ConfigClass::log() {
  Logger.logln("⚙ Stored configuration:");
  Logger.log("  • Device ID: ");
  Logger.logln(Helpers::getDeviceId());
  Logger.log("  • Configured: ");
  Logger.logln(this->_config_struct.configured ? "yes" : "no");
  Logger.log("  • Boot mode: ");
  switch (this->_config_struct.boot_mode) {
    case BOOT_CONFIG:
      Logger.logln("config");
      break;
    case BOOT_NORMAL:
      Logger.logln("normal");
      break;
    case BOOT_OTA:
      Logger.logln("OTA");
      break;
    default:
      Logger.logln("unknown");
      break;
  }
  Logger.log("  • Name: ");
  Logger.logln(this->_config_struct.name);

  Logger.logln("  • Wi-Fi");
  Logger.log("    • SSID: ");
  Logger.logln(this->_config_struct.wifi.ssid);
  Logger.logln("    • Password not shown");

  Logger.logln("  • MQTT");
  Logger.log("    • Host: ");
  Logger.logln(this->_config_struct.mqtt.host);
  Logger.log("    • Port: ");
  Logger.logln(String(this->_config_struct.mqtt.port));
  Logger.log("    • Auth: ");
  Logger.logln(this->_config_struct.mqtt.auth ? "yes" : "no");
  if (this->_config_struct.mqtt.auth) {
    Logger.log("    • Username: ");
    Logger.logln(this->_config_struct.mqtt.username);
    Logger.logln("    • Password not shown");
  }
  Logger.log("    • SSL: ");
  Logger.logln(this->_config_struct.mqtt.ssl ? "yes" : "no");
  if (this->_config_struct.mqtt.ssl) {
    Logger.log("    • Fingerprint: ");
    Logger.logln(this->_config_struct.mqtt.fingerprint);
  }

  Logger.logln("  • OTA");
  Logger.log("    • Enabled: ");
  Logger.logln(this->_config_struct.ota.enabled ? "yes" : "no");
  if (this->_config_struct.ota.enabled) {
    Logger.log("    • Host: ");
    Logger.logln(this->_config_struct.ota.host);
    Logger.log("    • Port: ");
    Logger.logln(String(this->_config_struct.ota.port));
    Logger.log("    • Path: ");
    Logger.logln(String(this->_config_struct.ota.path));
    Logger.log("    • SSL: ");
    Logger.logln(this->_config_struct.ota.ssl ? "yes" : "no");
    if (this->_config_struct.mqtt.ssl) {
      Logger.log("    • Fingerprint: ");
      Logger.logln(this->_config_struct.ota.fingerprint);
    }
  }
}

ConfigClass HomieInternals::Config;

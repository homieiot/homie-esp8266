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
  Logger.log("  • configured: ");
  Logger.logln(this->_config_struct.configured ? "yes" : "no");
  Logger.log("  • boot_mode: ");
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
  Logger.log("  • name: ");
  Logger.logln(this->_config_struct.name);

  Logger.logln("  • wifi");
  Logger.log("    • ssid: ");
  Logger.logln(this->_config_struct.wifi.ssid);
  Logger.logln("    • password not shown");

  Logger.logln("  • mqtt");
  Logger.log("    • host: ");
  Logger.logln(this->_config_struct.mqtt.host);
  Logger.log("    • port: ");
  Logger.logln(String(this->_config_struct.mqtt.port));
  Logger.log("    • auth: ");
  Logger.logln(this->_config_struct.mqtt.auth ? "yes" : "no");
  if (this->_config_struct.mqtt.auth) {
    Logger.log("    • username: ");
    Logger.logln(this->_config_struct.mqtt.username);
    Logger.logln("    • password not shown");
  }
  Logger.log("    • ssl: ");
  Logger.logln(this->_config_struct.mqtt.ssl ? "yes" : "no");
  if (this->_config_struct.mqtt.ssl) {
    Logger.log("    • fingerprint: ");
    Logger.logln(this->_config_struct.mqtt.fingerprint);
  }

  Logger.logln("  • ota");
  Logger.log("    • enabled: ");
  Logger.logln(this->_config_struct.ota.enabled ? "yes" : "no");
  if (this->_config_struct.ota.enabled) {
    Logger.log("    • host: ");
    Logger.logln(this->_config_struct.ota.host);
    Logger.log("    • port: ");
    Logger.logln(String(this->_config_struct.ota.port));
    Logger.log("    • path: ");
    Logger.logln(String(this->_config_struct.ota.path));
    Logger.log("    • ssl: ");
    Logger.logln(this->_config_struct.ota.ssl ? "yes" : "no");
    if (this->_config_struct.mqtt.ssl) {
      Logger.log("    • fingerprint: ");
      Logger.logln(this->_config_struct.ota.fingerprint);
    }
  }
}

ConfigClass HomieInternals::Config;

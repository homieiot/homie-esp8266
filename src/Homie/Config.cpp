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

  this->configured = this->_config_struct.configured;

  if (!this->configured) {
    return false;
  }

  this->boot_mode = this->_config_struct.boot_mode;

  if (this->boot_mode != BOOT_CONFIG && this->boot_mode != BOOT_NORMAL && this->boot_mode != BOOT_OTA) {
    return false;
  }

  this->hostname = this->_config_struct.hostname;
  this->wifi_ssid = this->_config_struct.wifi_ssid;
  this->wifi_password = this->_config_struct.wifi_password;
  this->homie_host = this->_config_struct.homie_host;

  return true;
}

void ConfigClass::save() {
  this->_eepromBegin();

  this->_config_struct.configured = this->configured;
  this->_config_struct.boot_mode = this->boot_mode;
  strcpy(this->_config_struct.hostname, this->hostname);
  strcpy(this->_config_struct.wifi_ssid, this->wifi_ssid);
  strcpy(this->_config_struct.wifi_password, this->wifi_password);
  strcpy(this->_config_struct.homie_host, this->homie_host);
  EEPROM.put(EEPROM_OFFSET, this->_config_struct);
  EEPROM.commit();
}

void ConfigClass::setCustomEepromSize(int count) {
  this->_custom_eeprom_size = count;
}

void ConfigClass::log() {
  Logger.logln("Conifg: ");
  Logger.log("  * configured: ");
  Logger.logln(String(this->configured));
  Logger.log("  * boot_mode: ");
  Logger.logln(String(this->boot_mode));
  Logger.log("  * hostname: ");
  Logger.logln(this->hostname);
  Logger.log("  * wifi_ssid: ");
  Logger.logln(this->wifi_ssid);
  Logger.log("  * wifi_password: ");
  Logger.logln(this->wifi_password);
  Logger.log("  * homie_host: ");
  Logger.logln(this->homie_host);
}

ConfigClass HomieInternals::Config;

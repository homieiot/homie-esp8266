#include "BootOta.hpp"

using namespace HomieInternals;

BootOta::BootOta()
: Boot("OTA")
{
}

BootOta::~BootOta() {
}

void BootOta::setup() {
  Boot::setup();

  WiFi.mode(WIFI_STA);

  int wifiAttempts = 1;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    if (wifiAttempts++ <= 3) {
      WiFi.begin(this->_interface->config->get().wifi.ssid, this->_interface->config->get().wifi.password);
      this->_interface->logger->log(F("↕ Connecting to Wi-Fi (attempt "));
      this->_interface->logger->log(wifiAttempts);
      this->_interface->logger->logln(F("/3)"));
    } else {
      this->_interface->logger->logln(F("✖ Connection failed"));
      this->_interface->logger->logln(F("↻ Rebooting into normal mode..."));
      this->_interface->config->setOtaMode(false);
      ESP.restart();
    }
  }
  this->_interface->logger->logln(F("✔ Connected to Wi-Fi"));

  const char* host = this->_interface->config->get().ota.server.host;
  unsigned int port = this->_interface->config->get().ota.server.port;
  if (this->_interface->config->get().mqtt.server.mdns.enabled) {
    this->_interface->logger->log(F("Querying mDNS service "));
    this->_interface->logger->logln(this->_interface->config->get().mqtt.server.mdns.service);
    MdnsQueryResult result = Helpers::mdnsQuery(this->_interface->config->get().mqtt.server.mdns.service);
    if (result.success) {
      host = result.ip.toString().c_str();
      port = result.port;
      this->_interface->logger->log(F("✔ "));
      this->_interface->logger->log(F(" service found at "));
      this->_interface->logger->log(host);
      this->_interface->logger->log(F(":"));
      this->_interface->logger->logln(port);
    } else {
      this->_interface->logger->logln(F("✖ Service not found"));
      ESP.restart();
    }
  }

  this->_interface->logger->logln(F("Starting OTA..."));


  char dataToPass[(MAX_DEVICE_ID_LENGTH - 1) + 1 + (MAX_FIRMWARE_NAME_LENGTH - 1) + 1 + (MAX_FIRMWARE_VERSION_LENGTH - 1) + 1 + (MAX_FIRMWARE_VERSION_LENGTH - 1) + 1];
  strcpy(dataToPass, this->_interface->config->get().deviceId);
  strcat(dataToPass, "=");
  strcat(dataToPass, this->_interface->firmware.name);
  strcat(dataToPass, "=");
  strcat(dataToPass, this->_interface->firmware.version);
  strcat(dataToPass, "=");
  strcat(dataToPass, this->_interface->config->getOtaVersion());
  t_httpUpdate_return result = ESPhttpUpdate.update(host, port, this->_interface->config->get().ota.path, dataToPass, this->_interface->config->get().ota.server.ssl.enabled, this->_interface->config->get().ota.server.ssl.fingerprint, false);
  switch (result) {
    case HTTP_UPDATE_FAILED:
      this->_interface->logger->logln(F("✖ Update failed"));
      break;
    case HTTP_UPDATE_NO_UPDATES:
      this->_interface->logger->logln(F("✖ No updates"));
      break;
    case HTTP_UPDATE_OK:
      this->_interface->logger->logln(F("✔ Success"));
      break;
  }

  this->_interface->logger->logln(F("↻ Rebooting into normal mode..."));
  this->_interface->config->setOtaMode(false);
  ESP.restart();
}

void BootOta::loop() {
  Boot::loop();
}

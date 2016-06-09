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
      WiFi.begin(_interface->config->get().wifi.ssid, _interface->config->get().wifi.password);
      _interface->logger->log(F("↕ Connecting to Wi-Fi (attempt "));
      _interface->logger->log(wifiAttempts);
      _interface->logger->logln(F("/3)"));
    } else {
      _interface->logger->logln(F("✖ Connection failed"));
      _interface->logger->logln(F("↻ Rebooting into normal mode..."));
      _interface->config->setOtaMode(false);
      ESP.restart();
    }
  }
  _interface->logger->logln(F("✔ Connected to Wi-Fi"));

  const char* host = _interface->config->get().ota.server.host;
  uint16_t port = _interface->config->get().ota.server.port;
  if (_interface->config->get().mqtt.server.mdns.enabled) {
    _interface->logger->log(F("Querying mDNS service "));
    _interface->logger->logln(_interface->config->get().mqtt.server.mdns.service);
    MdnsQueryResult result = Helpers::mdnsQuery(_interface->config->get().mqtt.server.mdns.service);
    if (result.success) {
      host = result.ip.toString().c_str();
      port = result.port;
      _interface->logger->log(F("✔ "));
      _interface->logger->log(F(" service found at "));
      _interface->logger->log(host);
      _interface->logger->log(F(":"));
      _interface->logger->logln(port);
    } else {
      _interface->logger->logln(F("✖ Service not found"));
      ESP.restart();
    }
  }

  _interface->logger->logln(F("Starting OTA..."));


  char dataToPass[(MAX_DEVICE_ID_LENGTH - 1) + 1 + (MAX_FIRMWARE_NAME_LENGTH - 1) + 1 + (MAX_FIRMWARE_VERSION_LENGTH - 1) + 1 + (MAX_FIRMWARE_VERSION_LENGTH - 1) + 1];
  strcpy(dataToPass, _interface->config->get().deviceId);
  strcat(dataToPass, "=");
  strcat(dataToPass, _interface->firmware.name);
  strcat(dataToPass, "=");
  strcat(dataToPass, _interface->firmware.version);
  strcat(dataToPass, "=");
  strcat(dataToPass, _interface->config->getOtaVersion());
  t_httpUpdate_return result = ESPhttpUpdate.update(host, port, _interface->config->get().ota.path, dataToPass, _interface->config->get().ota.server.ssl.enabled, _interface->config->get().ota.server.ssl.fingerprint, false);
  switch (result) {
    case HTTP_UPDATE_FAILED:
      _interface->logger->logln(F("✖ Update failed"));
      break;
    case HTTP_UPDATE_NO_UPDATES:
      _interface->logger->logln(F("✖ No updates"));
      break;
    case HTTP_UPDATE_OK:
      _interface->logger->logln(F("✔ Success"));
      break;
  }

  _interface->logger->logln(F("↻ Rebooting into normal mode..."));
  _interface->config->setOtaMode(false);
  ESP.restart();
}

void BootOta::loop() {
  Boot::loop();
}

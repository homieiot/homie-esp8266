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
      WiFi.begin(Config.get().wifi.ssid, Config.get().wifi.password);
      Logger.log(F("↕ Connecting to Wi-Fi (attempt "));
      Logger.log(wifiAttempts);
      Logger.logln(F("/3)"));
    } else {
      Logger.logln(F("✖ Connection failed"));
      Logger.logln(F("↻ Rebooting into normal mode..."));
      Config.setOtaMode(false);
      ESP.restart();
    }
  }
  Logger.logln(F("✔ Connected to Wi-Fi"));

  const char* host = Config.get().ota.server.host;
  unsigned int port = Config.get().ota.server.port;
  /*
  if (Config.get().ota.mdns) {
    Logger.log(F("Querying mDNS service "));
    Logger.logln(Config.get().ota.mdnsService);

    int n = MDNS.queryService(Config.get().ota.mdnsService, "tcp");
    if (n == 0) {
      Logger.logln(F("No services found"));
      Config.setOtaMode(false);
      ESP.restart();
    } else {
      Logger.log(n);
      Logger.logln(F(" service(s) found, using first"));
      host = MDNS.IP(0);
      port = MDNS.port(0);
    }
  } */

  Logger.logln(F("Starting OTA..."));


  char dataToPass[(MAX_DEVICE_ID_LENGTH - 1) + 1 + (MAX_FIRMWARE_NAME_LENGTH - 1) + 1 + (MAX_FIRMWARE_VERSION_LENGTH - 1) + 1 + (MAX_FIRMWARE_VERSION_LENGTH - 1) + 1];
  strcpy(dataToPass, Config.get().deviceId);
  strcat(dataToPass, "=");
  strcat(dataToPass, this->_interface->firmware.name);
  strcat(dataToPass, "=");
  strcat(dataToPass, this->_interface->firmware.version);
  strcat(dataToPass, "=");
  strcat(dataToPass, Config.getOtaVersion());
  t_httpUpdate_return result = ESPhttpUpdate.update(host, port, Config.get().ota.path, dataToPass, Config.get().ota.server.ssl.enabled, Config.get().ota.server.ssl.fingerprint, false);
  switch (result) {
    case HTTP_UPDATE_FAILED:
      Logger.logln(F("✖ Update failed"));
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Logger.logln(F("✖ No updates"));
      break;
    case HTTP_UPDATE_OK:
      Logger.logln(F("✔ Success"));
      break;
  }

  Logger.logln(F("↻ Rebooting into normal mode..."));
  Config.setOtaMode(false);
  ESP.restart();
}

void BootOta::loop() {
  Boot::loop();
}

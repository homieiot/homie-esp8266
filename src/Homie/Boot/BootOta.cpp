#include "BootOta.hpp"

using namespace HomieInternals;

BootOta::BootOta(SharedInterface* sharedInterface)
: Boot(sharedInterface, "OTA")
, _sharedInterface(sharedInterface)
{
}

BootOta::~BootOta() {
}

void BootOta::setup() {
  Boot::setup();

  WiFi.mode(WIFI_STA);

  WiFi.begin(Config.get().wifi.ssid, Config.get().wifi.password);

  int wifi_attempts = 1;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    if (wifi_attempts <= 3) {
      WiFi.begin(Config.get().wifi.ssid, Config.get().wifi.password);
      Logger.log("Retrying Wi-Fi connection");
      Logger.log(String(wifi_attempts));
      Logger.logln("/3");
      wifi_attempts++;
    } else {
      Logger.logln("✖ Connection failed, rebooting in normal mode...");
      Config.setOtaMode(false);
      ESP.restart();
    }
  }
  Logger.logln("✔ Connected to Wi-Fi");

  const char* host = Config.get().ota.host;
  uint16_t port = Config.get().ota.port;
  /*
  if (Config.get().ota.mdns) {
    Logger.log("Querying mDNS service ");
    Logger.logln(Config.get().ota.mdnsService);

    int n = MDNS.queryService(Config.get().ota.mdnsService, "tcp");
    if (n == 0) {
      Logger.logln("No services found");
      Config.setOtaMode(false);
      ESP.restart();
    } else {
      Logger.log(String(n));
      Logger.logln(" service(s) found, using first");
      host = MDNS.IP(0);
      port = MDNS.port(0);
    }
  } */

  Logger.logln("Starting OTA...");


  String dataToPass = Helpers.getDeviceId();
  dataToPass.reserve(1 + strlen(this->_sharedInterface->firmware.name) + 1 + strlen(this->_sharedInterface->firmware.version) + 1);
  dataToPass += '=';
  dataToPass += this->_sharedInterface->firmware.name;
  dataToPass += '@';
  dataToPass += this->_sharedInterface->firmware.version;
  t_httpUpdate_return ret = ESPhttpUpdate.update(host, port, Config.get().ota.path, dataToPass, Config.get().ota.ssl, Config.get().ota.fingerprint, false);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Logger.logln("✖ Update failed");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Logger.logln("✖ No updates");
      break;
    case HTTP_UPDATE_OK:
      Logger.logln("✔ Success, rebooting");
      break;
  }

  Config.setOtaMode(false);
  ESP.restart();
}

void BootOta::loop() {
  Boot::loop();
}

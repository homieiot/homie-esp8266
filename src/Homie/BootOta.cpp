#include "BootOta.hpp"

BootOta::BootOta(SharedInterface* shared_interface)
: Boot("OTA")
, _shared_interface(shared_interface)
{
}

BootOta::~BootOta() {
}

void BootOta::setup() {
  Boot::setup();

  WiFi.hostname(Config.hostname);
  WiFi.mode(WIFI_STA);

  WiFi.begin(Config.wifi_ssid, Config.wifi_password);

  int wifi_attempts = 1;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    if (wifi_attempts < 3) {
      WiFi.begin(Config.wifi_ssid, Config.wifi_password);
      Serial.println("Retrying connection...");
      wifi_attempts++;
    } else {
      Serial.println("Connection failed, rebooting in normal mode...");
      Config.boot_mode = BOOT_NORMAL;
      Config.save();

      ESP.restart();
    }
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println("Starting OTA...");

  String dataToPass = Config.hostname;
  dataToPass += '=';
  dataToPass += this->_shared_interface->version;
  //t_httpUpdate_return ret = ESPhttpUpdate.update(Config.homie_host, 80, "/ota", dataToPass);
  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.0.18", 3000, "/ota", dataToPass);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("Update failed");
      Config.boot_mode = BOOT_NORMAL;
      Config.save();

      ESP.restart();
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No updates");
      Config.boot_mode = BOOT_NORMAL;
      Config.save();

      ESP.restart();
      break;
    case HTTP_UPDATE_OK:
      Config.boot_mode = BOOT_NORMAL;
      Config.save();
      Serial.println("Successfully updated"); // may not be called, auto reboot

      ESP.restart();
      break;
  }
}

void BootOta::loop() {
  Boot::loop();
}

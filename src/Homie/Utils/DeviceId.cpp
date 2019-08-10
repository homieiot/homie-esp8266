#include "DeviceId.hpp"

using namespace HomieInternals;

char DeviceId::_deviceId[];  // need to define the static variable

#ifdef ESP32
void DeviceId::generate() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(DeviceId::_deviceId, MAX_MAC_STRING_LENGTH+1 , "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
#elif defined(ESP8266)
void DeviceId::generate() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(DeviceId::_deviceId, MAX_MAC_STRING_LENGTH, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
#endif // ESP32

const char* DeviceId::get() {
  return DeviceId::_deviceId;
}

#include "DeviceId.hpp"

using namespace HomieInternals;

char DeviceId::_deviceId[];  // need to define the static variable

void DeviceId::generate() {
  uint8_t mac[MAX_MAC_LENGTH];
  WiFi.macAddress(mac);
  Helpers::macToString(mac, DeviceId::_deviceId);
}

const char* DeviceId::get() {
  return DeviceId::_deviceId;
}

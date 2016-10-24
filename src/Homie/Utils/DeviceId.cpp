#include "DeviceId.hpp"

using namespace HomieInternals;

char DeviceId::_deviceId[] = "";  // need to define the static variable

void DeviceId::generate() {
  char flashChipId[6 + 1];
  sprintf(flashChipId, "%06x", ESP.getFlashChipId());

  sprintf(DeviceId::_deviceId, "%06x%s", ESP.getChipId(), flashChipId + strlen(flashChipId) - 2);
}

const char* DeviceId::get() {
  return DeviceId::_deviceId;
}

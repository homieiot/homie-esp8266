#include "Helpers.hpp"

using namespace HomieInternals;


String Helpers::getDeviceId() {
  char chip_id[6 + 1];
  sprintf(chip_id, "%06x", ESP.getChipId());
  char flash_chip_id[6 + 1];
  sprintf(flash_chip_id, "%06x", ESP.getFlashChipId());

  String truncated_flash_id = String(flash_chip_id);
  truncated_flash_id = truncated_flash_id.substring(4);

  String device_id = String(chip_id);
  device_id += truncated_flash_id;

  return device_id;
}

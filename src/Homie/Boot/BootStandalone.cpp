#include "BootStandalone.hpp"

using namespace HomieInternals;

BootStandalone::BootStandalone()
  : Boot("standalone") {
}

BootStandalone::~BootStandalone() {
}

void BootStandalone::setup() {
  Boot::setup();

  WiFi.mode(WIFI_OFF);

  ResetHandler::attach();
#if HOMIE_FIRMWARE_HOMIE_BUTTON
  HomieButton::attach();
#endif
}

void BootStandalone::loop() {
  Boot::loop();
}

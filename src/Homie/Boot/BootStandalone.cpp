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

#if HOMIE_CONFIG
  ResetHandler::Attach();
#endif
}

void BootStandalone::loop() {
  Boot::loop();
}

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

  ResetHandler::Attach();
}

void BootStandalone::loop() {
  Boot::loop();
}

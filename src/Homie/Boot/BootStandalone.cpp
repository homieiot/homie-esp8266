#include "BootStandalone.hpp"

using namespace HomieInternals;

BootStandalone::BootStandalone()
  : Boot("standalone")
  , ResetButton() {
}

BootStandalone::~BootStandalone() {
}

void BootStandalone::setup() {
  Boot::setup();

  WiFi.mode(WIFI_OFF);

  ResetButton::Attach();
}

void BootStandalone::loop() {
  Boot::loop();

  ResetButton::_handleReset();

  if (ResetButton::_flaggedForReset && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;
    Interface::get().getConfig().setHomieBootModeOnNextBoot(HomieBootMode::CONFIGURATION);

    Interface::get().getLogger() << F("Triggering ABOUT_TO_RESET event...") << endl;
    Interface::get().event.type = HomieEventType::ABOUT_TO_RESET;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().getLogger() << F("↻ Rebooting into config mode...") << endl;
    Serial.flush();
    ESP.restart();
  }
}

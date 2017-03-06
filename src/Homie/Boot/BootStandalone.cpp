#include "BootStandalone.hpp"

using namespace HomieInternals;

BootStandalone::BootStandalone()
: Boot("standalone")
, _flaggedForConfig(false) {
}

BootStandalone::~BootStandalone() {
}

void BootStandalone::_handleReset() {
  if (Interface::get().reset.enabled) {
    _resetDebouncer.update();

    if (_resetDebouncer.read() == Interface::get().reset.triggerState) {
      _flaggedForConfig = true;
      Interface::get().getLogger() << F("Flagged for configuration mode by pin") << endl;
    }
  }

  if (Interface::get().reset.flaggedBySketch) {
    _flaggedForConfig = true;
    Interface::get().getLogger() << F("Flagged for configuration mode by sketch") << endl;
  }
}

void BootStandalone::setup() {
  Boot::setup();

  WiFi.mode(WIFI_OFF);

  if (Interface::get().reset.enabled) {
    pinMode(Interface::get().reset.triggerPin, INPUT_PULLUP);

    _resetDebouncer.attach(Interface::get().reset.triggerPin);
    _resetDebouncer.interval(Interface::get().reset.triggerTime);
  }
}

void BootStandalone::loop() {
  Boot::loop();

  _handleReset();

  if (_flaggedForConfig && Interface::get().reset.idle) {
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

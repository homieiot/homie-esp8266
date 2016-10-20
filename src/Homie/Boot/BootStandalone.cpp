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
      Interface::get().logger->println(F("Flagged for configuration mode by pin"));
    }
  }

  if (Interface::get().reset.flaggedBySketch) {
    _flaggedForConfig = true;
    Interface::get().logger->println(F("Flagged for configuration mode by sketch"));
  }
}

void BootStandalone::setup() {
  Boot::setup();

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
    Interface::get().logger->println(F("Device is idle"));
    Interface::get().config->bypassStandalone();
    Interface::get().logger->println(F("Next reboot will bypass standalone mode"));

    Interface::get().logger->println(F("Triggering ABOUT_TO_RESET event..."));
    Interface::get().event.type = HomieEventType::ABOUT_TO_RESET;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().logger->println(F("↻ Rebooting into config mode..."));
    Serial.flush();
    ESP.restart();
  }
}

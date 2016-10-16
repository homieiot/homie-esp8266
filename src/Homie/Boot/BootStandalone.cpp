#include "BootStandalone.hpp"

using namespace HomieInternals;

BootStandalone::BootStandalone()
: Boot("standalone")
, _flaggedForConfig(false) {
}

BootStandalone::~BootStandalone() {
}

void BootStandalone::_handleReset() {
  if (_interface->reset.enabled) {
    _resetDebouncer.update();

    if (_resetDebouncer.read() == _interface->reset.triggerState) {
      _flaggedForConfig = true;
      _interface->logger->println(F("Flagged for configuration mode by pin"));
    }
  }

  if (_interface->reset.userFunction()) {
    _flaggedForConfig = true;
    _interface->logger->println(F("Flagged for configuration mode by function"));
  }
}

void BootStandalone::setup() {
  Boot::setup();

  if (_interface->reset.enabled) {
    pinMode(_interface->reset.triggerPin, INPUT_PULLUP);

    _resetDebouncer.attach(_interface->reset.triggerPin);
    _resetDebouncer.interval(_interface->reset.triggerTime);
  }
}

void BootStandalone::loop() {
  Boot::loop();

  _handleReset();

  if (_flaggedForConfig && _interface->reset.able) {
    _interface->logger->println(F("Device is in a resettable state"));
    _interface->config->bypassStandalone();
    _interface->logger->println(F("Next reboot will bypass standalone mode"));

    _interface->logger->println(F("Triggering ABOUT_TO_RESET event..."));
    _interface->event.type = HomieEventType::ABOUT_TO_RESET;
    _interface->eventHandler(_interface->event);

    _interface->logger->println(F("↻ Rebooting into config mode..."));
    Serial.flush();
    ESP.restart();
  }
}

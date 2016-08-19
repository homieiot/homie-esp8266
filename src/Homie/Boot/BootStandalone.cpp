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
      _interface->logger->logln(F("Flagged for configuration mode by pin"));
    }
  }

  if (_interface->reset.userFunction()) {
    _flaggedForConfig = true;
    _interface->logger->logln(F("Flagged for configuration mode by function"));
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
    _interface->logger->logln(F("Device is in a resettable state"));
    _interface->config->bypassStandalone();
    _interface->logger->logln(F("Next reboot will bypass standalone mode"));

    _interface->logger->logln(F("Triggering ABOUT_TO_RESET event..."));
    _interface->eventHandler(HomieEvent::ABOUT_TO_RESET);

    _interface->logger->logln(F("↻ Rebooting into config mode..."));
    _interface->logger->flush();
    ESP.restart();
  }
}

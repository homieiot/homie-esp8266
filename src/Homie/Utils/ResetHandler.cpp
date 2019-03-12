#include "ResetHandler.hpp"

#if HOMIE_CONFIG
using namespace HomieInternals;

Ticker ResetHandler::_resetBTNTicker;
Bounce ResetHandler::_resetBTNDebouncer;
Ticker ResetHandler::_resetTicker;
bool ResetHandler::_sentReset = false;

void ResetHandler::Attach() {
  if (Interface::get().reset.enabled) {
    pinMode(Interface::get().reset.triggerPin, INPUT_PULLUP);
    _resetBTNDebouncer.attach(Interface::get().reset.triggerPin);
    _resetBTNDebouncer.interval(Interface::get().reset.triggerTime);

    _resetBTNTicker.attach_ms(10, _tick);
    _resetTicker.attach_ms(100, _handleReset);
  }
}

void ResetHandler::_tick() {
  if (!Interface::get().reset.resetFlag && Interface::get().reset.enabled) {
    _resetBTNDebouncer.update();
    if (_resetBTNDebouncer.read() == Interface::get().reset.triggerState) {
      Interface::get().getLogger() << F("Flagged for reset by pin") << endl;
      Interface::get().disable = true;
      Interface::get().reset.resetFlag = true;
    }
  }
}

void ResetHandler::_handleReset() {
  if (Interface::get().reset.resetFlag && !_sentReset && Interface::get().reset.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;

    Interface::get().getConfig().erase();
    Interface::get().getLogger() << F("Configuration erased") << endl;

    // Set boot mode
    Interface::get().getConfig().setHomieBootModeOnNextBoot(HomieBootMode::CONFIGURATION);

    Interface::get().getLogger() << F("Triggering ABOUT_TO_RESET event...") << endl;
    Interface::get().event.type = HomieEventType::ABOUT_TO_RESET;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().getLogger() << F("↻ Rebooting into config mode...") << endl;
    Serial.flush();
    ESP.restart();
    _sentReset = true;
  }
}
#endif

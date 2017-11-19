#include "ResetButton.hpp"

using namespace HomieInternals;


bool ResetButton::_flaggedForReset = false;
Ticker ResetButton::_resetTicker;
Bounce ResetButton::_resetDebouncer;

void ResetButton::Attach()
{
  if (Interface::get().reset.enabled) {
    pinMode(Interface::get().reset.triggerPin, INPUT_PULLUP);

    _resetTicker.attach_ms(10, _handleReset);

    _resetDebouncer.attach(Interface::get().reset.triggerPin);
    _resetDebouncer.interval(Interface::get().reset.triggerTime);
  }
}

void ResetButton::_handleReset() {
  if (!_flaggedForReset && Interface::get().reset.enabled) {
    _resetDebouncer.update();

    if (_resetDebouncer.read() == Interface::get().reset.triggerState) {
      _flaggedForReset = true;
      Interface::get().getLogger() << F("Flagged for reset by pin") << endl;
      return; // allow other processes to deal with this flag if needed
    }
  }

  if (Interface::get().reset.flaggedBySketch) {
    _flaggedForReset = true;
    Interface::get().getLogger() << F("Flagged for reset by sketch") << endl;
    return; // allow other processes to deal with this flag if needed
  }

  if (_flaggedForReset && Interface::get().reset.idle) {
    _flaggedForReset = false;

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
  }
}

#include "ResetButton.hpp"

using namespace HomieInternals;



ResetButton::ResetButton()
  : _flaggedForReset(false) {
}

void ResetButton::Attach()
{
  if (Interface::get().reset.enabled) {
    pinMode(Interface::get().reset.triggerPin, INPUT_PULLUP);

    _resetDebouncer.attach(Interface::get().reset.triggerPin);
    _resetDebouncer.interval(Interface::get().reset.triggerTime);
  }
}

void ResetButton::_handleReset() {
  if (Interface::get().reset.enabled) {
    _resetDebouncer.update();

    if (_resetDebouncer.read() == Interface::get().reset.triggerState) {
      _flaggedForReset = true;
      Interface::get().getLogger() << F("Flagged for reset by pin") << endl;
    }
  }

  if (Interface::get().reset.flaggedBySketch) {
    _flaggedForReset = true;
    Interface::get().getLogger() << F("Flagged for reset by sketch") << endl;
  }
}

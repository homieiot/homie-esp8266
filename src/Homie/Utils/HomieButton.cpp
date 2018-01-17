#include "HomieButton.hpp"

using namespace HomieInternals;

Ticker HomieButton::_homieButtonTicker;
OneButton* HomieButton::_homieButton;
CallbackFunction HomieButton::_userClickFunc = [](){};

void HomieButton::attach() {
    Interface::get().getLogger() << F("Homie button is enabled") << endl;

    _homieButtonTicker.attach_ms(10, _tick);

    _homieButton = new OneButton(Interface::get().reset.triggerPin, !Interface::get().reset.triggerState);
    _homieButton->setPressTicks(Interface::get().reset.triggerTime);

    _homieButton->attachClick(_clickFunc);
    _homieButton->attachDoubleClick(_doubleClickFunc);
    _homieButton->attachLongPressStart(_longPressStartFunc);
}

void HomieButton::addClickHandler(CallbackFunction func) {
  _userClickFunc = func;
}

void HomieButton::_tick() {
  _homieButton->tick();
}

void HomieButton::_clickFunc() {
  _userClickFunc();
}

void HomieButton::_doubleClickFunc() {
  // change Modes
  if (Interface::get().bootMode == HomieBootMode::NORMAL) {
    Interface::get().getConfig().setHomieBootModeOnNextBoot(HomieBootMode::CONFIGURATION);
    Interface::get().getLogger() << F("Changed next boot mode to congig") << endl;
    Interface::get().flags.disable = true;
    Interface::get().flags.reboot = true;
  } else if (Interface::get().bootMode == HomieBootMode::CONFIGURATION) {
    Interface::get().getConfig().setHomieBootModeOnNextBoot(HomieBootMode::NORMAL);
    Interface::get().getLogger() << F("Changed next boot mode to normal") << endl;
    Interface::get().flags.disable = true;
    Interface::get().flags.reboot = true;
  }
}

void HomieButton::_longPressStartFunc() {
  // reset device
  if (Interface::get().reset.enabled && !Interface::get().flags.reset) {
    Interface::get().getLogger() << F("Flagged for reset by pin") << endl;
    Interface::get().flags.disable = true;
    Interface::get().flags.reset = true;
  }
}

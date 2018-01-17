#include "ResetHandler.hpp"

using namespace HomieInternals;

Ticker ResetHandler::_resetTicker;
bool ResetHandler::_sentReset = false;
bool ResetHandler::_sentRestart = false;

void ResetHandler::attach() {
  _resetTicker.attach_ms(100, _tick);
}

void ResetHandler::_tick() {
  _handleReboot();
  _handleReset();
}

void ResetHandler::_handleReboot() {
  if (Interface::get().flags.reboot && !_sentRestart && Interface::get().flags.idle) {
    Interface::get().getLogger() << F("Device is idle") << endl;

    Interface::get().getLogger() << F("Triggering ABOUT_TO_RESTART event...") << endl;
    Interface::get().event.type = HomieEventType::ABOUT_TO_RESTART;
    Interface::get().eventHandler(Interface::get().event);

    Interface::get().getLogger() << F("↻ Rebooting...") << endl;
    Serial.flush();
    ESP.restart();
    _sentRestart = true;
  }
}

void ResetHandler::_handleReset() {
  if (Interface::get().flags.reset && !_sentReset && Interface::get().flags.idle) {
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

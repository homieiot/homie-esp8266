/*

# Homie enabled Sonoff Dual shutters

Requires the Shutters library:
https://github.com/marvinroger/arduino-shutters
and the SonoffDual library:
https://github.com/marvinroger/arduino-sonoff-dual

## Features

* Do a short press to close shutters
if level != 0 or open shutters if level == 0
* Do a long press to reset

*/

#include <Homie.h>

#include <EEPROM.h>
#include <SonoffDual.h>
#include <Shutters.h>

const unsigned long UPCOURSE_TIME = 30 * 1000;
const unsigned long DOWNCOURSE_TIME = 30 * 1000;
const float CALIBRATION_RATIO = 0.1;

const bool RELAY1_MOVE = true;
const bool RELAY1_STOP = false;

const bool RELAY2_UP = true;
const bool RELAY2_DOWN = false;

const byte SHUTTERS_EEPROM_POSITION = 0;

HomieNode shuttersNode("shutters", "shutters");

// Shutters

void shuttersOperationHandler(Shutters *shutters, ShuttersOperation operation)
{
  switch (operation)
  {
  case ShuttersOperation::UP:
    SonoffDual.setRelays(RELAY1_MOVE, RELAY2_UP);
    break;
  case ShuttersOperation::DOWN:
    SonoffDual.setRelays(RELAY1_MOVE, RELAY2_DOWN);
    break;
  case ShuttersOperation::HALT:
    SonoffDual.setRelays(RELAY1_STOP, false);
    break;
  }
}

void shuttersWriteStateHandler(Shutters *shutters, const char *state, byte length)
{
  for (byte i = 0; i < length; i++)
  {
    EEPROM.write(SHUTTERS_EEPROM_POSITION + i, state[i]);
#ifdef ESP8266
    EEPROM.commit();
#endif
  }
}

void readFromEeprom(char *dest, byte length)
{
  for (byte i = 0; i < length; i++)
  {
    dest[i] = EEPROM.read(SHUTTERS_EEPROM_POSITION + i);
  }
}

void onShuttersLevelReached(Shutters *shutters, uint8_t level)
{
  if (shutters->isIdle())
    Homie.setIdle(true); // if idle, we've reached our target
  if (Homie.isConnected())
    shuttersNode.setProperty("level").send(String(level));
}

Shutters shutters;

// Homie

void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  case HomieEventType::ABOUT_TO_RESET:
    shutters.reset();
    break;
  default:
    break;
  }
}

bool shuttersLevelHandler(const HomieRange &range, const String &value)
{
  for (byte i = 0; i < value.length(); i++)
  {
    if (isDigit(value.charAt(i)) == false)
      return false;
  }

  const unsigned long numericValue = value.toInt();
  if (numericValue > 100)
    return false;

  // wanted value is valid

  if (shutters.isIdle() && numericValue == shutters.getCurrentLevel())
    return true; // nothing to do

  Homie.setIdle(false);
  shutters.setLevel(numericValue);

  return true;
}

// Logic

void setup()
{
  SonoffDual.setup();
  EEPROM.begin(4);
  shutters.begin();

  Homie_setFirmware("sonoff-dual-shutters", "1.0.0");
  Homie.disableLogging();
  Homie.disableResetTrigger();
  Homie.setLedPin(SonoffDual.LED_PIN, SonoffDual.LED_ON);
  Homie.onEvent(onHomieEvent);

  char storedShuttersState[shutters.getStateLength()];
  readFromEeprom(storedShuttersState, shutters.getStateLength());

  shutters
      .setCourseTime(UPCOURSE_TIME, DOWNCOURSE_TIME)
      .setCalibrationRatio(CALIBRATION_RATIO)
      .setOperationHandler(shuttersOperationHandler)
      .setWriteStateHandler(shuttersWriteStateHandler)
      .onLevelReached(onShuttersLevelReached)
      .restoreState(storedShuttersState)
      .begin();

  shuttersNode.advertise("level").settable(shuttersLevelHandler);

  Homie.setup();
}

void loop()
{
  shutters.loop();
  Homie.loop();
  SonoffDualButton buttonState = SonoffDual.handleButton();
  if (buttonState == SonoffDualButton::LONG)
  {
    Homie.reset();
  }
  else if (buttonState == SonoffDualButton::SHORT && shutters.isIdle())
  {
    Homie.setIdle(false);

    if (shutters.getCurrentLevel() == 100)
    {
      shutters.setLevel(0);
    }
    else
    {
      shutters.setLevel(100);
    }
  }
}

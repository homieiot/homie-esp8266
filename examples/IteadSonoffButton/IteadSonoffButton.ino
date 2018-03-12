/*
 *   Tested with "WiFi Smart Socket ESP8266 MQTT"
 *   and "Sonoff - WiFi Wireless Smart Switch ESP8266 MQTT"
 *
 *   The Relay could be toggeled with the physical pushbutton
*/

#include <Homie.h>
#include <SwitchNode.hpp>

#define PIN_RELAY 12
#define PIN_LED 13
#define EXT_BUTTON_PIN 0
#define EXT_BUTTON_ACTIVE_STATE 1

bool relayState;

OneButton externalButton(EXT_BUTTON_PIN, EXT_BUTTON_ACTIVE_STATE);
SwitchNode relayNode("relay");

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(PIN_RELAY, OUTPUT);
  setRelayState(LOW);

  Homie_setFirmware("itead-sonoff-buton", "1.0.0");
  Homie.setLedPin(PIN_LED, LOW);

  relayNode.setCallback([](const bool val) {
    setRelayState(val);
  });
  externalButton.attachClick([]() {
    setRelayState(!relayState, true);
  });

  Homie.setup();
}

void loop() {
  Homie.loop();
  externalButton.tick();
}

void setRelayState(const bool val,  const bool overwriteSetter = false) {
  relayState = val;
  digitalWrite(PIN_RELAY, relayState);
  relayNode.setValue(relayState, overwriteSetter);
  Homie.getLogger() << "Switch is " << (relayState ? "on" : "off") << endl;
}

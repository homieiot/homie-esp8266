/* WARNING: untested */

#include <Homie.h>

const int PIN_RELAY = 12;
const int PIN_LED = 13;
const int PIN_BUTTON = 0;

HomieNode switchNode("switch", "Switch", "switch");

bool switchOnHandler(const HomieRange& range, const String& value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  switchNode.setProperty("on").send(value);
  Homie.getLogger() << "Switch is " << (on ? "on" : "off") << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  Homie_setFirmware("itead-sonoff", "1.0.0");
  Homie.setLedPin(PIN_LED, LOW).setResetTrigger(PIN_BUTTON, LOW, 5000);

  switchNode.advertise("on").setName("On").setDatatype("boolean").settable(switchOnHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}

#include <Homie.h>

const unsigned int NUMBER_OF_LED = 4;
const unsigned char LED_PINS[NUMBER_OF_LED] = { 16, 5, 4, 0 };

HomieNode stripNode("strip", "Strip", "strip", true, 1, NUMBER_OF_LED);

bool stripLedHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range

  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range

  if (value != "on" && value != "off") return false;  // if the value is not valid

  bool on = (value == "on");

  digitalWrite(LED_PINS[range.index - 1], on ? HIGH : LOW);
  stripNode.setProperty("led").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Led " << range.index << " is " << value << endl;

  return true;
}

void setup() {
  for (int i = 0; i < NUMBER_OF_LED; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("awesome-ledstrip", "1.0.0");

  stripNode.advertise("led").setName("Led").setDatatype("boolean").settable(stripLedHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}

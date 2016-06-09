#include <Homie.h>

const uint8_t NUMBER_OF_LED = 4;
const uint8_t LED_PINS[NUMBER_OF_LED] = { 16, 5, 4, 0 };

bool stripHandler(String, String); // forward declaration (needed for Arduino <= 1.6.8)
HomieNode stripNode("ledstrip", "ledstrip", stripHandler);

bool stripHandler(String property, String value) {
  for (int i = 0; i < property.length(); i++) {
    if (!isDigit(property.charAt(i))) {
      return false;
    }
  }

  int ledIndex = property.toInt();
  if (ledIndex < 0 || ledIndex > NUMBER_OF_LED - 1) {
    return false;
  }

  if (value == "true") {
    digitalWrite(LED_PINS[ledIndex], HIGH);
    Homie.setNodeProperty(stripNode, String(ledIndex), "true"); // Update the state of the led
    Serial.print("Led ");
    Serial.print(ledIndex);
    Serial.println(" is on");
  } else if (value == "false") {
    digitalWrite(LED_PINS[ledIndex], LOW);
    Homie.setNodeProperty(stripNode, String(ledIndex), "false");
    Serial.print("Led ");
    Serial.print(ledIndex);
    Serial.println(" is off");
  } else {
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  for (int i = 0; i < NUMBER_OF_LED; i++) {
    pinMode(LED_PINS[i], INPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  Homie_setFirmware("awesome-ledstrip", "1.0.0");
  stripNode.subscribeToAll();
  Homie.setup();
}

void loop() {
  Homie.loop();
}

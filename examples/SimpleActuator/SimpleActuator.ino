#include <Homie.h>

const int PIN_RELAY = D1;

HomieNode light("light", "light");

void inputHandler(String node, String property, String message) {
  if (node != "light" || property != "on") {
    return;
  }

  if (message == "true") {
    digitalWrite(PIN_RELAY, HIGH);
    Homie.setNodeProperty(light, "on", "true"); // Update the state of the light
  } else if (message == "false") {
    digitalWrite(PIN_RELAY, LOW);
    Homie.setNodeProperty(light, "on", "false");
  }
}

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  Homie.setFirmware("awesome-relay" ,"1.0.0");
  light.subscribe("on");
  Homie.registerNode(light);
  Homie.setInputHandler(inputHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

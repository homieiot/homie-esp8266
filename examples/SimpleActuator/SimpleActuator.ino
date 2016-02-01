#include <Homie.h>

const int PIN_RELAY = D1;

HomieNode light("light", "light");

bool lightOnHandler(String message) {
  if (message == "true") {
    digitalWrite(PIN_RELAY, HIGH);
    Homie.setNodeProperty(light, "on", "true"); // Update the state of the light
    Serial.println("Light is on");
  } else if (message == "false") {
    digitalWrite(PIN_RELAY, LOW);
    Homie.setNodeProperty(light, "on", "false");
    Serial.println("Light is off");
  } else {
    return false;
  }

  return true;
}

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  Homie.setFirmware("awesome-relay" ,"1.0.0");
  light.subscribe("on", lightOnHandler);
  Homie.registerNode(light);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

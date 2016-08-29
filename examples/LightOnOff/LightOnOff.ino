#include <Homie.h>

const int PIN_RELAY = 5;

HomieNode lightNode("light", "switch");

bool lightOnHandler(HomieRange range, String value) {
  if (value == "true") {
    digitalWrite(PIN_RELAY, HIGH);
    Homie.setNodeProperty(lightNode, "on").send("true");
    Serial.println("Light is on");
  } else if (value == "false") {
    digitalWrite(PIN_RELAY, LOW);
    Homie.setNodeProperty(lightNode, "on").send("false");
    Serial.println("Light is off");
  } else {
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  Homie_setFirmware("awesome-relay", "1.0.0");

  lightNode.advertise("on")->settable(lightOnHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}

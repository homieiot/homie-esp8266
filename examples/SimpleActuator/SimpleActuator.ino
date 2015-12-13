#include <Homie.h>

const int PIN_RELAY = D1;

void inputHandler(String node, String property, String message) {
  if (node != "light" || property != "on") {
    return;
  }

  if (message == "true") {
    digitalWrite(PIN_RELAY, HIGH);
  } else if (message == "false") {
    digitalWrite(PIN_RELAY, LOW);
  }
}

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  Homie.setVersion("1.0.0");
  Homie.addNode("light", "light");
  Homie.addSubscription("light", "on");
  Homie.setInputHandler(inputHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

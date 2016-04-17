#include <Homie.h>

const int PIN_DOOR = 16;

Bounce debouncer = Bounce(); // Bounce is built into Homie, so you can use it without including it first
int lastDoorValue = -1;

HomieNode doorNode("door", "door");

void loopHandler() {
  int doorValue = debouncer.read();

  if (doorValue != lastDoorValue) {
     Serial.print("Door is now: ");
     Serial.println(doorValue ? "open" : "close");

     if (Homie.setNodeProperty(doorNode, "open", doorValue ? "true" : "false", true)) {
       lastDoorValue = doorValue;
     } else {
       Serial.println("Sending failed");
     }
  }
}

void setup() {
  pinMode(PIN_DOOR, INPUT);
  digitalWrite(PIN_DOOR, HIGH);
  debouncer.attach(PIN_DOOR);
  debouncer.interval(50);

  Homie.setFirmware("awesome-door", "1.0.0");
  Homie.registerNode(doorNode);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
  debouncer.update();
}

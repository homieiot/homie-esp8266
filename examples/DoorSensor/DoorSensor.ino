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

     Homie.setNodeProperty(doorNode, "open", doorValue ? "true" : "false");
     lastDoorValue = doorValue;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(PIN_DOOR, INPUT);
  digitalWrite(PIN_DOOR, HIGH);
  debouncer.attach(PIN_DOOR);
  debouncer.interval(50);

  Homie_setFirmware("awesome-door", "1.0.0");
  Homie.setLoopFunction(loopHandler);

  doorNode.advertise("open");

  Homie.setup();
}

void loop() {
  Homie.loop();
  debouncer.update();
}

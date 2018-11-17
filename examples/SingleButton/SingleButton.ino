#include <Homie.h>

const int PIN_BUTTON = 5; // D1

int buttonState = 0;
bool buttonPressed = false;

HomieNode buttonNode("button", "Button", "button");

void loopHandler() {
  buttonState = !digitalRead(PIN_BUTTON);

  if (buttonState == HIGH && !buttonPressed) {
    buttonNode.setProperty("button").send("PRESSED");
    Homie.getLogger() << "Button pressed" << endl;
    buttonPressed = true;
  } else if (buttonState == LOW && buttonPressed) {
    buttonNode.setProperty("button").send("RELEASED");
    Homie.getLogger() << "Button released" << endl;
    buttonPressed = false;
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("awesome-button", "1.0.0");
  Homie.setLoopFunction(loopHandler);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  buttonNode.advertise("button").setName("Button")
                                .setDatatype("enum")
                                .setFormat("PRESSED,RELEASED")
                                .setRetained(false);

  Homie.setup();
}

void loop() {
  Homie.loop();
}

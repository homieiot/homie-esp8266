#include <Homie.h>

HomieNode lightNode("light", "switch");

bool globalInputHandler(const HomieNode& node, String property, HomieRange range, String value) {
  Serial << "Received on node " << node.getId() << ": " << property << " = " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("global-input-handler", "1.0.0");
  Homie.setGlobalInputHandler(globalInputHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}

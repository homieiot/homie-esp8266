#include <Homie.h>

HomieNode lightNode("light", "switch");

bool globalInputHandler(const HomieNode& node, const String& property, const HomieRange& range, const String& value) {
  Homie.getLogger() << "Received on node " << node.getId() << ": " << property << " = " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("global-input-handler", "1.0.0");
  Homie.setGlobalInputHandler(globalInputHandler);

  lightNode.advertise("on").settable();

  Homie.setup();
}

void loop() {
  Homie.loop();
}

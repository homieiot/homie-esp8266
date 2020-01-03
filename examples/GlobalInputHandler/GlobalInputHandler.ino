#include <Homie.h>

HomieNode lightNode("light", "Light", "switch");

bool globalInputHandler(const HomieNode& node, const HomieRange& range, const String& property, const String& value) {
  Homie.getLogger() << "Received on node " << node.getId() << ": " << property << " = " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("global-input-handler", "1.0.0");
  Homie.setGlobalInputHandler(globalInputHandler);

  lightNode.advertise("on").setName("On").setDatatype("boolean").settable();

  Homie.setup();
}

void loop() {
  Homie.loop();
}

/*

This is a sketch testing all the different types of handlers
The global input handler will always be triggered.
Setting lightnode1/property1/ will trigger the node input handler.
Setting lightnode2/property1/ will trigger the property input handler.

*/

#include <Arduino.h>
#include <Homie.h>

bool globalInputHandler(const HomieNode& node, const HomieRange& range, const String& property, const String& value) {
  Homie.getLogger() << "Global input handler. Received on node " << node.getId() << ": " << property << " = " << value << endl;
  return false;
}

bool nodeInputHandler(const HomieRange & range, const String & property, const String & value) {
  Homie.getLogger() << "Node input handler. Received on property " << property << " value: " << value;
  return true;
}

bool propertyInputHandler(const HomieRange& range, const String& value) {
  Homie.getLogger() << "Property input handler. Receveived value: " << value;
  return true;
}

HomieNode lightNode1("lightnode1", "Light Node One Name","switch", false, 0 , 0, &nodeInputHandler);
HomieNode lightNode2("lightnode2", "Light Two One Name","switch");

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("Test all input handlers", "0.0.1");
  lightNode1.advertise("property1").setName("ln1 First property").setDatatype("boolean").settable();
  lightNode2.advertise("property1").setName("ln2 First property").setDatatype("boolean").settable(propertyInputHandler);
  Homie.setGlobalInputHandler(globalInputHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

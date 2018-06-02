#include <Homie.h>

bool broadcastHandler(const String& level, const String& value) {
  Homie.getLogger() << "Received broadcast level " << level << ": " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("broadcast-test", "1.0.0");
  Homie.setBroadcastHandler(broadcastHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}

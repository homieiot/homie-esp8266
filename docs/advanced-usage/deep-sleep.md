Before deep sleeping, you will want to ensure that all messages are sent, including the `$online →  false`. To do that, you can call `Homie.prepareToSleep()`. This will disconnect everything cleanly, so that you can call `ESP.deepSleep()`.

```c++
#include <Homie.h>

void onHomieEvent(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::MQTT_READY:
      Homie.getLogger() << "MQTT connected, preparing for deep sleep..." << endl;
      Homie.prepareToSleep();
      break;
    case HomieEventType::READY_TO_SLEEP:
      Homie.getLogger() << "Ready to sleep" << endl;
      Homie.doDeepSleep();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
```

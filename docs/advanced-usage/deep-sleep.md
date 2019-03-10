Before deep sleeping, you will want to ensure that all messages are sent, including property values of your `HomieNode` and the `$online →  false`. To do that, you can call `Homie.prepareToSleep()`. But the event `MQTT_READY` is emitted before your `HomieNode`s `loop()` method is called. Therefore you need to postpone the call to `Homie.prepareToSleep()` until the `loop()` method of all `HomieNode`s is called and their properties are submitted over MQTT. For that the additional Timer is necessary. Then `Homie.prepareToSleep()` will disconnect everything cleanly, so that you can call `Homie.doDeepSleep()`.

```c++
#include <Homie.h>
#include "Timer.h"

Timer t;

void prepareSleep() {
  Homie.prepareToSleep();
}

void onHomieEvent(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::MQTT_READY:
      Homie.getLogger() << "MQTT connected, preparing for deep sleep after 100ms..." << endl;
      t.after(100, prepareSleep);
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
  t.update();
}
```

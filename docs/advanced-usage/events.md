# Setup() and Loop()

It is possible to register `setup()` and `loop()` functions:
```c++
    Homie_setFirmware("device", "1.0.0");  // The underscore is not a typo! See Magic bytes
    Homie.setSetupFunction(setupHandler);  // before Homie.setup()
    Homie.setLoopFunction(loopHandler);    // before Homie.setup()

    Homie.setup();
```

These functions have to be set before calling `setup()`. 

* The function passed in `setLoopFunction()` is called from Homie.loop() when MQTT is connected.
* The function passed in `setSetupfunction()` is called after the first MQTT connection has been made.

# Global Event Handler

You may want to hook to Homie events. Maybe you will want to control an RGB LED if the Wi-Fi connection is lost, or execute some code prior to a device reset, for example to clear some EEPROM you're using:

```c++
void onHomieEvent(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::STANDALONE_MODE:
      // Do whatever you want when standalone mode is started
      break;
    case HomieEventType::CONFIGURATION_MODE:
      // Do whatever you want when configuration mode is started
      break;
    case HomieEventType::NORMAL_MODE:
      // Do whatever you want when normal mode is started
      break;
    case HomieEventType::OTA_STARTED:
      // Do whatever you want when OTA is started
      break;
    case HomieEventType::OTA_PROGRESS:
      // Do whatever you want when OTA is in progress

      // You can use event.sizeDone and event.sizeTotal
      break;
    case HomieEventType::OTA_FAILED:
      // Do whatever you want when OTA is failed
      break;
    case HomieEventType::OTA_SUCCESSFUL:
      // Do whatever you want when OTA is successful
      break;
    case HomieEventType::ABOUT_TO_RESET:
      // Do whatever you want when the device is about to reset
      break;
    case HomieEventType::WIFI_CONNECTED:
      // Do whatever you want when Wi-Fi is connected in normal mode

      // You can use event.ip, event.gateway, event.mask
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      // Do whatever you want when Wi-Fi is disconnected in normal mode

      // You can use event.wifiReason
      break;
    case HomieEventType::MQTT_READY:
      // Do whatever you want when MQTT is connected in normal mode
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      // Do whatever you want when MQTT is disconnected in normal mode

      // You can use event.mqttReason
      break;
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      // Do whatever you want when an MQTT packet with QoS > 0 is acknowledged by the broker

      // You can use event.packetId
      break;
    case HomieEventType::READY_TO_SLEEP:
      // After you've called `prepareToSleep()`, the event is triggered when MQTT is disconnected
      break;
    case HomieEventType::SENDING_STATISTICS:
      // Do whatever you want when statistics are sent in normal mode
      break;
  }
}

void setup() {
  Homie.onEvent(onHomieEvent); // before Homie.setup()
  // ...
}
```

See the following example for a concrete use case:

[![GitHub logo](../assets/github.png) HookToEvents.ino](https://github.com/homieiot/homie-esp8266/blob/develop/examples/HookToEvents/HookToEvents.ino)

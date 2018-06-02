By default, Homie for ESP8266 will blink the built-in LED to indicate its status. Note it does not indicate activity, only the status of the device (in `configuration` mode, connecting to Wi-Fi or connecting to MQTT), see [Getting started](../quickstart/getting-started.md) for more information.

However, on some boards like the ESP-01, the built-in LED is actually the TX port, so it is fine if Serial is not enabled, but if you enable Serial, this is a problem. You can easily disable the built-in LED blinking.

```c++
void setup() {
  Homie.disableLedFeedback(); // before Homie.setup()
  // ...
}
```

You may, instead of completely disable the LED control, set a new LED to control:

```c++
void setup() {
  Homie.setLedPin(16, HIGH); // before Homie.setup() -- 2nd param is the state of the pin when the LED is o
  // ...
}
```

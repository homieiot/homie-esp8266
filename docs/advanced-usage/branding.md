By default, Homie for ESP8266 will spawn an `Homie-xxxxxxxxxxxx` AP and will connect to the MQTT broker with the `Homie-xxxxxxxxxxxx` client ID. You might want to change the `Homie` text:

```c++
void setup() {
  Homie_setBrand("MyIoTSystem"); // before Homie.setup()
  // ...
}
```

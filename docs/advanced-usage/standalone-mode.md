Homie for ESP8266 has a special mode named `standalone`. It was a [requested feature](https://github.com/homieiot/homie-esp8266/issues/125) to implement a way not to boot into `configuration` mode on initial boot, so that a device can work without being configured first. It was already possible in `configuration` mode, but the device would spawn an AP which would make it insecure.

To enable this mode, call `Homie.setStandalone()`:

```c++
void setup() {
  Homie.setStandalone(); // before Homie.setup()
  // ...
}
```

To actually configure the device, you have to reset it, the same way you would to go from `normal` mode to `configuration` mode.

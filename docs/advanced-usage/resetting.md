Resetting the device means erasing the stored configuration and rebooting from `normal` mode to `configuration` mode. By default, you can do it by pressing for 5 seconds the `FLASH` button of your ESP8266 board.

This behavior is configurable:

```c++
void setup() {
  Homie.setResetTrigger(1, LOW, 2000); // before Homie.setup()
  // ...
}
```

The device will now reset if pin `1` is `LOW` for `2000`ms. You can also disable completely this reset trigger:

```c++
void setup() {
  Homie.disableResetTrigger(); // before Homie.setup()
  // ...
}
```

In addition, you can also trigger a device reset from your sketch:

```c++
void loop() {
  Homie.reset();
}
```

This will reset the device as soon as it is idle. Indeed, sometimes, you might want to disable temporarily the ability to reset the device. For example, if your device is doing some background work like moving shutters, you will want to disable the ability to reset until the shutters are not moving anymore.

```c++
Homie.setIdle(false);
```

Note that if a reset is asked while the device is not idle, the device will be flagged. In other words, when you will call `Homie.setIdle(true);` back, the device will immediately reset.

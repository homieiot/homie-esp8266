## 1. I see some garbage on the Serial monitor?

You are probably using a generic ESP8266. The problem with these modules is the built-in LED is tied to the serial line. You can do two things:

* Disable the serial logging, to have the LED working:

```c++
void setup() {
  Homie.enableLogging(false); // before Homie.setup()
  // ...
}
```

* Disable the LED blinking, to have the serial line working:

```c++
void setup() {
  Homie.enableBuiltInLedIndicator(false); // before Homie.setup()
  // ...
}
```

## 2. I see an `abort` message on the Serial monitor?

`abort()` is called by Homie for ESP8266 when the framework is used in a bad way. The possible causes are:

* You are calling a function that is meant to be called before `Homie.setup()`, after `Homie.setup()`

* One of the string you've used (in `setFirmware()`, `subscribe()`, etc.) is too long. Check the `Limits.hpp` file to see the max length possible for each string.

## 3. The network is completely unstable... What's going on?

The framework needs to work continuously (ie. `Homie.loop()` needs to be called very frequently). In other words, don't use `delay()` (see [avoid delay](http://playground.arduino.cc/Code/AvoidDelay)) or anything that might block the code for more than 50ms or so. There is also a known Arduino for ESP8266 issue with `analogRead()`, see [Limitations and known issues](limitations-and-known-issues.md#adc-readings).

## 4. My device resets itself without me doing anything?

You have probably connected a sensor to the default reset pin of the framework (D3 on NodeMCU, GPIO0 on other boards). See [Resetting](../advanced-usage/resetting.md).

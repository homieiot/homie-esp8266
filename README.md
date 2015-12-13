# Homie

![Homie logo](logo.png)

An opiniated IoT framework for the ESP8266.

## Features

* Easy credentials configuration through a Web UI
* Network tolerant with automatic reconnection
* OTA support
* Pretty straightforward sketches, a simple light for example:

```c++
#include <Homie.h>

const int PIN_RELAY = D1;

void inputHandler(String node, String property, String message) {
  if (node != "light" || property != "on") {
    return;
  }

  if (message == "true") {
    digitalWrite(PIN_RELAY, HIGH);
  } else if (message == "false") {
    digitalWrite(PIN_RELAY, LOW);
  }
}

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  Homie.setVersion("1.0.0");
  Homie.addNode("light", "light");
  Homie.addSubscription("light", "on");
  Homie.setInputHandler(inputHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
```

## Requirements

* An ESP8266
* The [Arduino IDE for ESP8266](https://github.com/esp8266/Arduino) (version 2.0.0 minimum)

## Installation

1. Download the [latest version](https://github.com/marvinroger/homie/archive/master.zip)
2. Load the `.zip` with **Sketch → Include Library → Add .ZIP Library**

## How does it work?

An Homie device has 3 modes of operation:

1. The `config` mode is the initial one. It spawns an AP and a JSON webserver. A client can connect to it, get the list of available networks, and send the credentials (Wi-Fi SSID, password, and the Homie server hostname/IP). Once the Homie device receives the credentials, it boots into normal mode.

2. The `normal` mode is the mode the device will be most of the time. It connects to the Wi-Fi, to the MQTT, it sends initial informations to the Homie server (like the local IP, the version of the firmware...) and it subscribes from the server to the subscriptions given (see `addSubscription()`) and to OTA events. It then runs the given `setup` function (see `setSetupFunction()`), and loops the given `loop` function (see `setLoopFunction()`). This provides a nice abstraction, as everything is handled in the lower level. If there is a network failure, it will try to auto-reconnect, endlessly. To return to the `config` mode, the user has to press the `FLASH` button of the ESP8266 board for 5 seconds.

3. The `OTA` mode is triggered from the `normal` mode when the Homie server sends a version newer than the current firmware version (see `setVersion()`). It will reach the Homie server and download the latest firmware available. When it ends (either a success or a failure), it returns to `normal` mode.

## API

You don't have to instanciate an `Homie` instance, it is done internally.

See examples folder for examples.

### Core functions

#### void .setup ()

Setup Homie. Must be called once in `setup()`, after Homie is configured (`addNode`, `addSubscription`...) and when you have set the initial state of GPIOs.

#### void .loop ()

Handle Homie work. Must be called in `loop()`.

**Never call `delay()` in the sketch loop() as it will be blocking, causing Homie to malfunction.**

### Configuration functions

#### void .setVersion (const char\* `version`)

Set the version of the sketch. This is useful for OTA, as Homie will check against the server if there is a newer version.

* **`version`**: Version of the sketch. Default value is `undefined`

#### void .addNode (const char\* `name`, const char\* `type`)

Add a node to the device. A device might have multiple nodes, like a temperature one and an humidity one. See the Wiki to see all node types and the properties it exposes.

* **`name`**: Name of the node
* **`type`**: Type of the node. See the Wiki

#### void .addSubscription (const char \* `node`, const char\* `property`)

Subscribe to a property. A device might want to subscribe to a property (like `level` for a `shutters` node for example).

* **`node`**: Name of the node
* **`property`**: Property to listen

#### void .setInputHandler (void (\*`callback`)(String `node`, String `property`, String `message`))

Set input handler for subscribed properties.

* **`callback`**: Input handler
* **`node`**: Name of the node
* **`preoperty`**: Name of the property
* **`message`**: Payload

#### void .setResetFunction (void (\*`callback`)(void))

Set the reset function. If you use a library that uses EEPROM for example, you will want to provide a function that clears the EEPROM.

* **`callback`**: Reset function

#### void .setResettable (bool `resettable`)

Is the device resettable? This is useful at runtime, because you might want the device not to be resettable when you have another library that is doing some unfinished work, like moving shutters for example.

* **`resettable`**: Is the device resettable? Default value is `true`

#### void .setSetupFunction (void (\*`callback`)(void))

You can provide the function that will be called when operating in normal mode (when the device is configured and connected).

* **`callback`**: Setup function

#### void .setLoopFunction (void (\*`callback`)(void))

You can provide the function that will be looped in normal mode.

* **`callback`**: Loop function

#### void .reserveEeprom (int `bytes`)

Homie uses EEPROM internally, and the ESP8266 needs to know how many bytes are going to be used. So you can reserve how many bytes you will use in EEPROM.

* **`bytes`**: Number of bytes you will use

### Runtime functions

#### void .sendProperty (String `node`, String `property`, String `value`, bool `retained` = true)

Using this function, you can send a value to a property, like a temperature for example.

* **`node`**: Name of the node
* **`property`**: Property to send
* **`value`**: Payload
* **`retained`**: Should the server retain this value, or is it a one-shot value?

#### int .getEepromOffset ()

As Homie uses EEPROM, first bytes of the EEPROM are used. This functions returns the first byte that is free for you to use.

#### bool .readyToOperate ()

Is the device in normal mode, configured and connected? You should not need this function. But maybe you will.

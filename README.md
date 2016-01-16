Homie for ESP8266
=================

An opinionated IoT framework for the ESP8266.

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
* The [Arduino IDE for ESP8266](https://github.com/esp8266/Arduino) (version 2.1.0 minimum)

## Installation

1. Download the [latest version](https://github.com/marvinroger/homie/archive/master.zip)
2. Load the `.zip` with **Sketch → Include Library → Add .ZIP Library**

## How does it work?

An Homie device has 3 modes of operation:

1. The `config` mode is the initial one. It spawns an AP and a JSON webserver. A client can connect to it, get the list of available networks, and send the credentials (Wi-Fi SSID, password, and the Homie server hostname/IP). There is a client available at [marvinroger/homie-esp8266-configurator](https://github.com/marvinroger/homie-esp8266-configurator). See [Configuration API](#configuration-api) if you want to implement your own client. Once the Homie device receives the credentials, it boots into normal mode.

2. The `normal` mode is the mode the device will be most of the time. It connects to the Wi-Fi, to the MQTT, it sends initial informations to the Homie server (like the local IP, the version of the firmware...) and it subscribes from the server to the subscriptions given (see `addSubscription()`) and to OTA events. It then runs the given `setup` function (see `setSetupFunction()`), and loops the given `loop` function (see `setLoopFunction()`). This provides a nice abstraction, as everything is handled in the lower level. If there is a network failure, it will try to auto-reconnect, endlessly. To return to the `config` mode, the user has to press the `FLASH` button of the ESP8266 board for 5 seconds.

3. The `OTA` mode is triggered from the `normal` mode when the Homie server sends a version newer than the current firmware version (see `setVersion()`). It will reach the Homie server and download the latest firmware available. When it ends (either a success or a failure), it returns to `normal` mode.

## API

You don't have to instantiate an `Homie` instance, it is done internally.

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

## Configuration API

When in `config` mode, the device spawns an open AP named `Homie-XXXXXXXX`. To configure it, you need to connect to this network.

The device exposes a CORS-compliant HTTP server available at `http://homie.config` on port `80`. To bypass the built-in DNS server, you can also reach the server at `192.168.1.1`.

### HTTP API

#### GET `/heart`

This is useful to ping the server to ensure the device is started.

##### Response

200 OK (application/json)

```json
{ "heart": "beat" }
```

#### GET `/networks`

Retrieve the Wi-Fi networks the device can see.

##### Response

200 OK (application/json)

```json
{
  "networks": [
    { "ssid": "Network_2", "rssi": -82, "encryption": "wep" },
    { "ssid": "Network_1", "rssi": -57, "encryption": "wpa" },
    { "ssid": "Network_3", "rssi": -65, "encryption": "wpa2" },
    { "ssid": "Network_5", "rssi": -94, "encryption": "none" },
    { "ssid": "Network_4", "rssi": -89, "encryption": "auto" }
  ]
}
```

#### PUT `/config`

Save the config to the device.

##### Request body

(application/json)

```json
{
  "name": "kitchen-light",
  "wifi_ssid": "Network_1",
  "wifi_password": "I'm a Wi-Fi password!",
  "homie_host": "192.168.1.10"
}
```

`wifi_password` can be left blank if open network.

##### Response

200 OK (application/json)

```json
{ "success": true }
```

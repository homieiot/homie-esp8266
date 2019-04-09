This *Getting Started* guide assumes you have an ESP8266 board with an user-configurable LED, and an user programmable button, like a NodeMCU DevKit 1.0, for example. These restrictions can be lifted (see next pages).

To use Homie for ESP8266, you will need:

* An ESP8266
* The Arduino IDE for ESP8266 (version 2.3.0 minimum)
* Basic knowledge of the Arduino environment (upload a sketch, import libraries, ...)
* To understand [the Homie convention](https://github.com/homieiot/convention)

## Installing Homie for ESP8266

There are two ways to install Homie for ESP8266.

### 1a. For the Arduino IDE

There is a YouTube video with instructions:

[![YouTube logo](../assets/youtube.png) How to install Homie libraries on Arduino IDE](https://www.youtube.com/watch?v=bH3KfFfYUvg)

1. Download the [release corresponding to this documentation version](https://github.com/homieiot/homie-esp8266/releases)

2. Load the `.zip` with **Sketch → Include Library → Add .ZIP Library**

Homie for ESP8266 has 5 dependencies:

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) >= 5.0.8
* [Bounce2](https://github.com/thomasfredericks/Bounce2)
* [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) >= [c8ed544](https://github.com/me-no-dev/ESPAsyncTCP)
* [AsyncMqttClient](https://github.com/marvinroger/async-mqtt-client)
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

Some of them are available through the Arduino IDE, with **Sketch → Include Library → Manage Libraries**. For the others, install it by downloading the `.zip` on GitHub.

### 1b. With [PlatformIO](http://platformio.org)

In a terminal, run `platformio lib install 555`.

!!! warning "Not yet released as stable"
    The above command is for when the v2 is stable and released. Currently, the latest stable version is 1.5. In the meantime, use the develop branch to get started with the v2, add this in your **platformio.ini**:

    ```
    lib_deps = git+https://github.com/homieiot/homie-esp8266.git#develop
    ```

Dependencies are installed automatically.

## Bare minimum sketch

```c++
#include <Homie.h>

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("bare-minimum", "1.0.0"); // The underscore is not a typo! See Magic bytes
  Homie.setup();
}

void loop() {
  Homie.loop();
}
```


This is the bare minimum needed for Homie for ESP8266 to work correctly.

!!! tip "LED"
    ![Solid LED](../assets/led_solid.gif)
    If you upload this sketch, you will notice the LED of the ESP8266 will light on. This is because you are in `configuration` mode.

Homie for ESP8266 has 3 modes of operation:

1. By default, the `configuration` mode is the initial one. It spawns an AP and an HTTP webserver exposing a JSON API. To interact with it, you have to connect to the AP. Then, an HTTP client can get the list of available Wi-Fi networks and send the configuration (like the Wi-Fi SSID, the Wi-Fi password, some settings...). Once the device receives the credentials, it boots into `normal` mode.

2. The `normal` mode is the mode the device will be most of the time. It connects to the Wi-Fi, to the MQTT, it sends initial informations to the Homie server (like the local IP, the version of the firmware currently running...) and it subscribes to the needed MQTT topics. It automatically reconnects to the Wi-Fi and the MQTT when the connection is lost. It also handle the OTA. The device can return to `configuration` mode in different ways (press of a button or custom function, see [Resetting](../advanced-usage/resetting.md)).

3. The `standalone` mode. See [Standalone mode](../advanced-usage/standalone-mode.md).

!!! warning
    **As a rule of thumb, never block the device with blocking code for more than 50ms or so.** Otherwise, you may very probably experience unexpected behaviors.

## Connecting to the AP and configuring the device

Homie for ESP8266 has spawned a secure AP named `Homie-xxxxxxxxxxxx`, like `Homie-c631f278df44`. Connect to it.

!!! tip "Hardware device ID"
    This `c631f278df44` ID is unique to each device, and you cannot change it (this is actually the MAC address of the station mode). If you flash a new sketch, this ID won't change.

Once connected, the webserver is available at `http://192.168.123.1`. Every domain name is resolved by the built-in DNS server to this address. You can then configure the device using the [HTTP JSON API](../configuration/http-json-api.md). When the device receives its configuration, it will reboot into `normal` mode.

## Understanding what happens in `normal` mode

### Visual codes

When the device boots in `normal` mode, it will start blinking:

!!! tip "LED"
    ![Slowly blinking LED](../assets/led_wifi.gif)
    Slowly when connecting to the Wi-Fi

!!! tip "LED"
    ![Fast blinking LED](../assets/led_mqtt.gif)
    Faster when connecting to the MQTT broker

This way, you can have a quick feedback on what's going on. If both connections are established, the LED will stay off. Note the device will also blink during the automatic reconnection, if the connection to the Wi-Fi or the MQTT broker is lost.

### Under the hood

Although the sketch looks like it does not do anything, it actually does quite a lot:

* It automatically connects to the Wi-Fi and MQTT broker. No more network boilerplate code
* It exposes the Homie device on MQTT (as `<base topic>/<device ID>`, e.g. `homie/c631f278df44`)
* It subscribes to the special OTA and configuration topics, automatically flashing a sketch if available or updating the configuration
* It checks for a button press on the ESP8266, to return to `configuration` mode

## Creating an useful sketch

Now that we understand how Homie for ESP8266 works, let's create an useful sketch. We want to create a smart light.

[![GitHub logo](../assets/github.png) LightOnOff.ino](https://github.com/homieiot/homie-esp8266/blob/develop/examples/LightOnOff/LightOnOff.ino)

Alright, step by step:

1. We create a node with an ID of `light`, and name `Light` and a type of `switch` with `HomieNode lightNode("light", "Light", "switch")`
2. We set the name and the version of the firmware with `Homie_setFirmware("awesome-light" ,"1.0.0");`
3. We want our `light` node to advertise an `on` property, which is settable. We do that with `lightNode.advertise("on").settable(lightOnHandler);`. The `lightOnHandler` function will be called when the value of this property is changed
4. In the `lightOnHandler` function, we want to update the state of the `light` node. We do this with `lightNode.setProperty("on").send("true");`

In about thirty SLOC, we have achieved to create a smart light, without any hard-coded credentials, with automatic reconnection in case of network failure, and with OTA support. Not bad, right?

## Creating a sensor node

In the previous example sketch, we were reacting to property changes. But what if we want, for example, to send a temperature every 5 minutes? We could do this in the Arduino `loop()` function. But then, we would have to check if we are in `normal` mode, and we would have to ensure the network connection is up before being able to send anything. Boring.

Fortunately, Homie for ESP8266 provides an easy way to do that.

[![GitHub logo](../assets/github.png) TemperatureSensor.ino](https://github.com/homieiot/homie-esp8266/blob/develop/examples/TemperatureSensor/TemperatureSensor.ino)

The only new things here are the `Homie.setSetupFunction(setupHandler);` and `Homie.setLoopFunction(loopHandler);` calls. The setup function will be called once, when the device is in `normal` mode and the network connection is up. The loop function will be called everytime, when the device is in `normal` mode and the network connection is up. This provides a nice level of abstraction.

Now that you understand the basic usage of Homie for ESP8266, you can head on to the next pages to learn about more powerful features like input handlers, the event system and custom settings.

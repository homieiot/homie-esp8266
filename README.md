![homie-esp8266 banner](banner.png)

Homie for ESP8266
=================

[![Build Status](https://img.shields.io/travis/marvinroger/homie-esp8266/develop.svg?style=flat-square)](https://travis-ci.org/marvinroger/homie-esp8266) [![Latest Release](https://img.shields.io/badge/release-v2.0.0-yellow.svg?style=flat-square)](https://github.com/marvinroger/homie-esp8266/releases)

An Arduino for ESP8266 implementation of [Homie](https://github.com/marvinroger/homie), an MQTT convention for the IoT.

## Download

The Git repository contains the development version of Homie for ESP8266. Stable releases are available [on the releases page](https://github.com/marvinroger/homie-esp8266/releases).

## Features

* Automatic connection/reconnection to Wi-Fi/MQTT
* [JSON configuration file](https://homie-esp8266.readme.io/v2.0.0/docs/json-configuration-file) to configure the device
* [Cute HTTP API / Web UI / App](https://homie-esp8266.readme.io/v2.0.0/docs/http-json-api) to remotely send the configuration to the device and get information about it
* [Custom settings](https://homie-esp8266.readme.io/v2.0.0/docs/custom-settings)
* [OTA over MQTT](https://homie-esp8266.readme.io/v2.0.0/docs/ota-configuration-updates)
* [Magic bytes](https://homie-esp8266.readme.io/v2.0.0/docs/magic-bytes)
* Available in the [PlatformIO registry](http://platformio.org/#!/lib/show/555/Homie)
* Pretty straightforward sketches, a simple light for example:

```c++
#include <Homie.h>

const int PIN_RELAY = 5;

HomieNode lightNode("light", "switch");

bool lightOnHandler(HomieRange range, String value) {
  if (value == "true") {
    digitalWrite(PIN_RELAY, HIGH);
    Homie.setNodeProperty(lightNode, "on", "true"); // Update the state of the light
    Serial.println("Light is on");
  } else if (value == "false") {
    digitalWrite(PIN_RELAY, LOW);
    Homie.setNodeProperty(lightNode, "on", "false");
    Serial.println("Light is off");
  } else {
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  Homie_setFirmware("awesome-relay", "1.0.0");

  lightNode.advertise("on").settable(lightOnHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
```

## Requirements, installation and usage

The project is documented on https://homie-esp8266.readme.io with a *Getting started* guide and every piece of information you will need.

![homie-esp8266 banner](banner.png)

Homie for ESP8266
=================

[![Build Status](https://img.shields.io/circleci/project/github/homieiot/homie-esp8266/develop.svg?style=flat-square)](https://circleci.com/gh/homieiot/homie-esp8266) [![Latest Release](https://img.shields.io/badge/release-v2.0.0-yellow.svg?style=flat-square)](https://github.com/homieiot/homie-esp8266/releases) [![Gitter](https://img.shields.io/gitter/room/Homie/ESP8266.svg?style=flat-square)](https://gitter.im/homie-iot/ESP8266) [![PlatformIO](https://img.shields.io/badge/Powered-PlatformIO-blue.png)](https://platformio.org/lib/show/555/Homie)

An Arduino for ESP8266 implementation of [Homie](https://github.com/homieiot/convention), an MQTT convention for the IoT.

Currently Homie for ESP8266 implements [Homie 2.0.1](https://github.com/homieiot/convention/releases/tag/v2.0.1)

## Note about v3 features (Auto-discovery, ESP32, ...)

Version v3 is currently in development. It adds support for Homie-convention v3.0.1 and ESP32 controllers.

If you are interested in testing or helping development, please try the [#develop-v3 branch](https://github.com/homieiot/homie-esp8266/tree/develop-v3).

## Note for v1.x users

The old configurator is not available online anymore. You can download it [here](https://github.com/homieiot/homie-esp8266/releases/download/v1.5.0/homie-esp8266-v1-setup.zip).

## Download

The Git repository contains the development version of Homie for ESP8266. Stable releases are available [on the releases page](https://github.com/homieiot/homie-esp8266/releases).

## Using with PlatformIO

[PlatformIO](http://platformio.org) is an open source ecosystem for IoT development with cross platform build system, library manager and full support for Espressif ESP8266 development. It works on the popular host OS: Mac OS X, Windows, Linux 32/64, Linux ARM (like Raspberry Pi, BeagleBone, CubieBoard).

1. Install [PlatformIO IDE](http://platformio.org/platformio-ide)
2. Create new project using "PlatformIO Home > New Project"
3. Open [Project Configuration File `platformio.ini`](http://docs.platformio.org/page/projectconf.html)

### Stable version

4. Add "Homie" to project using `platformio.ini` and [lib_deps](http://docs.platformio.org/page/projectconf/section_env_library.html#lib-deps) option:
```ini
[env:myboard]
platform = espressif8266
board = ...
framework = arduino
build_flags = -D PIO_FRAMEWORK_ARDUINO_LWIP2_LOW_MEMORY
lib_deps = Homie
```

Add the `PIO_FRAMEWORK_ARDUINO_LWIP2_LOW_MEMORY` build flag to ensure reliable OTA updates.

### Development version

4. Update dev/platform to staging version:
   - [Instruction for Espressif 8266](http://docs.platformio.org/en/latest/platforms/espressif8266.html#using-arduino-framework-with-staging-version)

5. Before editing platformio.ini as shown below, you must install "git" if you don't already have it. For Windows, just go to http://git-scm.com/download/win and the download will start automatically. Note, this is only a requirement for the development versions.

6. Add development version of "Homie" to project using `platformio.ini` and [lib_deps](http://docs.platformio.org/page/projectconf/section_env_library.html#lib-deps) option:
```ini
[env:myboard]
platform = ...
board = ...
framework = arduino
build_flags = -D PIO_FRAMEWORK_ARDUINO_LWIP2_LOW_MEMORY

; the latest development branch (convention V2)
lib_deps = https://github.com/homieiot/homie-esp8266.git#develop

; the latest development branch (convention V3.0.x) 
lib_deps = https://github.com/homieiot/homie-esp8266.git#develop-v3

; or tagged version
lib_deps = https://github.com/homieiot/homie-esp8266.git#v2.0.0-beta.2
```

-----
Happy coding with PlatformIO!

## Features

* Automatic connection/reconnection to Wi-Fi/MQTT
* [JSON configuration file](http://homieiot.github.io/homie-esp8266/docs/develop/configuration/json-configuration-file) to configure the device
* [Cute HTTP API / Web UI / App](http://homieiot.github.io/homie-esp8266/docs/develop/configuration/http-json-api) to remotely send the configuration to the device and get information about it
* [Custom settings](http://homieiot.github.io/homie-esp8266/docs/develop/advanced-usage/custom-settings)
* [OTA over MQTT](http://homieiot.github.io/homie-esp8266/docs/develop/others/ota-configuration-updates)
* [Magic bytes](http://homieiot.github.io/homie-esp8266/docs/develop/advanced-usage/magic-bytes)
* Available in the [PlatformIO registry](http://platformio.org/#!/lib/show/555/Homie)
* Pretty straightforward sketches, a simple light for example:

```c++
#include <Homie.h>

const int PIN_RELAY = 5;

HomieNode lightNode("light", "switch");

bool lightOnHandler(const HomieRange& range, const String& value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  lightNode.setProperty("on").send(value);
  Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
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

The project is documented on https://homieiot.github.io/homie-esp8266/ with a *Getting started* guide and every piece of information you will need.

## Donate

I am a student and maintaining Homie for ESP8266 takes time. **I am not in need and I will continue to maintain this project as much as I can even without donations**. Consider this as a way to tip the project if you like it. :wink:

[![Donate button](https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=JSGTYJPMNRC74)

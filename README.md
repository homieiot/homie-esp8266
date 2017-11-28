![homie-esp8266 banner](banner.png)

Homie for ESP8266
=================

[![Build Status](https://img.shields.io/circleci/project/github/marvinroger/homie-esp8266/develop.svg?style=flat-square)](https://circleci.com/gh/marvinroger/homie-esp8266) [![Latest Release](https://img.shields.io/badge/release-v2.0.0-yellow.svg?style=flat-square)](https://github.com/marvinroger/homie-esp8266/releases) [![Gitter](https://img.shields.io/gitter/room/Homie/ESP8266.svg?style=flat-square)](https://gitter.im/homie-iot/ESP8266)

An Arduino for ESP8266 implementation of [Homie](https://github.com/marvinroger/homie), an MQTT convention for the IoT.

## Note for v1.x users

The old configurator is not available online anymore. You can download it [here](https://github.com/marvinroger/homie-esp8266/releases/download/v1.5.0/homie-esp8266-v1-setup.zip).

## Download

The Git repository contains the development version of Homie for ESP8266. Stable releases are available [on the releases page](https://github.com/marvinroger/homie-esp8266/releases).

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
lib_deps = Homie
```

### Development version

4. Update dev/platform to staging version:
   - [Instruction for Espressif 8266](http://docs.platformio.org/en/latest/platforms/espressif8266.html#using-arduino-framework-with-staging-version)
5. Add development version of "Homie" to project using `platformio.ini` and [lib_deps](http://docs.platformio.org/page/projectconf/section_env_library.html#lib-deps) option:
```ini
[env:myboard]
platform = ...
board = ...
framework = arduino

; the latest development branch
lib_deps = https://github.com/marvinroger/homie-esp8266.git

; or tagged version
lib_deps = https://github.com/marvinroger/homie-esp8266.git#v2.0.0-beta.1
```

-----
Happy coding with PlatformIO!

## Features

* Automatic connection/reconnection to Wi-Fi/MQTT
* [JSON configuration file](http://marvinroger.github.io/homie-esp8266/develop/configuration/json-configuration-file) to configure the device
* [Cute HTTP API / Web UI / App](http://marvinroger.github.io/homie-esp8266/develop/configuration/http-json-api) to remotely send the configuration to the device and get information about it
* [Custom settings](http://marvinroger.github.io/homie-esp8266/develop/advanced-usage/custom-settings)
* [OTA over MQTT](http://marvinroger.github.io/homie-esp8266/develop/others/ota-configuration-updates)
* [Magic bytes](http://marvinroger.github.io/homie-esp8266/develop/advanced-usage/magic-bytes)
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

The project is documented on http://marvinroger.github.io/homie-esp8266/ with a *Getting started* guide and every piece of information you will need.

## Donate

I am a student and maintaining Homie for ESP8266 takes time. **I am not in need and I will continue to maintain this project as much as I can even without donations**. Consider this as a way to tip the project if you like it. :wink:

[![Donate button](https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=JSGTYJPMNRC74)

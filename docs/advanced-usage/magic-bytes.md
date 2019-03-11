Homie for ESP8266 firmwares contain magic bytes allowing you to check if a firmware is actually an Homie for ESP8266 firmware, and if so, to get the name, the version and the brand of the firmware.

You might be wondering why `Homie_setFirmware()` instead of `Homie.setFirmware()`, this is because we use [special macros](https://github.com/homieiot/homie-esp8266/blob/8935639bc649a6c71ce817ea4f732988506d020e/src/Homie.hpp#L23-L24) to embed the magic bytes.

Values are encoded as such within the firmware binary:

Type | Left boundary | Value | Right boundary
---- | ------------- | ----- | --------------
Homie magic bytes | *None* | `0x25 0x48 0x4F 0x4D 0x49 0x45 0x5F 0x45 0x53 0x50 0x38 0x32 0x36 0x36 0x5F 0x46 0x57 0x25` | *None*
Firmware name | `0xBF 0x84 0xE4 0x13 0x54` | **actual firmware name** | `0x93 0x44 0x6B 0xA7 0x75`
Firmware version | `0x6A 0x3F 0x3E 0x0E 0xE1` | **actual firmware version** | `0xB0 0x30 0x48 0xD4 0x1A`
Firmware brand (only present if `Homie_setBrand()` called, Homie otherwise) | `0xFB 0x2A 0xF5 0x68 0xC0` | **actual firmware brand** | `0x6E 0x2F 0x0F 0xEB 0x2D`

See the following script for a concrete use case:

[![GitHub logo](../assets/github.png) firmware_parser.py](https://github.com/homieiot/homie-esp8266/blob/develop/scripts/firmware_parser)

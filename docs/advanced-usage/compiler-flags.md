Compiler flags can be used to remove certain functionality from homie-esp8266. This can become useful if your firmware gets too large in size and is not upgradable any more over OTA. On the other hand these compiler flags allow to remove features from homie-esp8266, that you do not require or do not use.

**HOMIE_CONFIG**

This compiler flag allows to disable the configuration mode completely. To configure your homie-esp8266, you need to upload the configuration to the SPIFFS before starting the device. Without a proper configuration the device will just restart after writing the error message about the missing configuration to the logger. Add the following to your platformio.ini file:

```
build_flags = -D HOMIE_CONFIG=0
```

This reduces the firmware size by about 50000 bytes.

**HOMIE_MDNS**

This compiler flag allows to disable the publishing of the device identifier via mDNS protocol. Add the following to your platformio.ini file:

```
build_flags = -D HOMIE_MDNS=0
```

This reduces the firmware size by about 6400 bytes.



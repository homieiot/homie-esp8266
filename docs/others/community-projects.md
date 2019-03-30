This page lists the projects made by the community to work with Homie.

# [jpmens/homie-ota](https://github.com/jpmens/homie-ota)

homie-ota is written in Python. It provides an OTA server for Homie devices as well as a simple inventory which can be useful to keep track of Homie devices. homie-ota also enables you to trigger an OTA update (over MQTT, using the Homie convention) from within its inventory. New firmware can be uploaded to homie-ota which detects firmware name (fwname) and version (fwversion) from the uploaded binary blob, thanks to an idea and code contributed by Marvin.

# [stufisher/homie-control](https://github.com/stufisher/homie-control)

homie-control provides a web UI to manage Homie devices as well as a series of virtual python devices to allow extended functionality.

Its lets you do useful things like:

* Historically log device properties
* Schedule changes in event properties (i.e. water your garden once a day)
* Execute profiles of property values (i.e. turn a series of lights on and off simultaneously)
* Trigger property changes based on:
   * When a network device is dis/connected (i.e. your phone joins your wifi, turn the lights on)
   * Sunset / rise
   * When another property changes

# [skoona/HomieMonitor](https://github.com/skoona/HomieMonitor)

This application is designed to act as a Homie Controller, or Monitor, in support of IOT/Devices using Homie-esp8266; although any Homie Device implementation should be supported.

* Monitor Homie V2, and V3 Devices (Initial Focus on `ESP8266`)
* Controller model for ESP8266 devices
* MQTT OTA operations
* *Discover Devices page* auto-refreshes every 30 seconds
* Packaged app
  * Self contained Application packaged as Java Executable warFile; using Warbler.gem -- port 8080
  * [Docker image](https://hub.docker.com/r/skoona/homie-monitor).
* Internally designed to tollerate potentially Homie Specification 1.5+, but focused on V3.
* Attribute and Property retention as Homie may consider some discovery related attributes optional and they are not always retained!
* MQTT Retained Message cleanup. Old/stale device topics retained in MQTT can be deleted to cleanup the discovery process.

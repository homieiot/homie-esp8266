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

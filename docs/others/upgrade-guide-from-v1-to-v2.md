This is an upgrade guide to upgrade your Homie devices from v1 to v2.

## New convention

The Homie convention has been revised to v2 to be more extensible and introspectable. Be sure to [check it out](https://github.com/homieiot/convention/tree/v2.0.0).

## API changes in the sketch

1. `Homie.setFirmware(name, version)` must be replaced by `Homie_setFirmware(name, version)`
2. `Homie.setBrand(brand)` must be replaced by `Homie_setBrand(brand)`
3. `Homie.registerNode()` must be removed, nodes are now automagically registered
4. If you've enabled Serial logging, `Serial.begin()` must be called explicitely in your sketch
5. Remove the `HOMIE_OTA_MODE` in your event handler, if you have one
6. The `Homie.setNodeProperty()` signature changed completely. If you had `Homie.setNodeProperty(node, "property", "value", true)`, the new equivalent syntax is `Homie.setNodeProperty(node, "property").setRetained(true).send("value")`. Note the `setRetained()` is not even required as messages are retained by default.
7. TODO

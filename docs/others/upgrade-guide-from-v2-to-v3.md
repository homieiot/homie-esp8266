This is an upgrade guide to upgrade your Homie devices from v2 to v3.

## New convention

The Homie convention has been revised to v3 to be more extensible and introspectable. Be sure to [check it out](https://github.com/homieiot/convention/tree/v3.0.1).

## API changes in the sketch

1. Constructor of `HomieNode` needs third mandatory param `const char* type`:
   E.g. `HomieNode lightNode("light", "Light");` -> `HomieNode lightNode("light", "Light", "switch");`.
2. Signature of handleInput has changed to: `handleInput(const HomieRange& range, const String& property, const String& value)`
   TODO: see Ping example.
3. TODO: The `loop`s of HomieNodes could be run although there is no MQTT connection by setting param ...
4. TODO

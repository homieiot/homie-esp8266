Homie for ESP8266 lets you implement custom settings that can be set from the JSON configuration file and the Configuration API. Below is an example of how to use this feature:

```c++
HomieSetting<long> percentageSetting("percentage", "A simple percentage");  // id, description

void setup() {
  percentageSetting.setDefaultValue(50).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= 100);
  });

  Homie.setup();
}
```

!!! tip "setDefaultValue() before Homie.setup()"
    As shown in the example above, the **default value** has to be set **before** `Homie.setup()` is called.
    Otherwise you get an error on startup if there is also no value configured in JSON configuration file.

An `HomieSetting` instance can be of the following types:

Type | Value
---- | -----
`bool` | `true` or `false`
`long` | An integer from `-2,147,483,648` to `2,147,483,647`
`double` | A floating number that can fit into a `real64_t`
`const char*` | Any string

By default, a setting is mandatory (you have to set it in the configuration file). If you give it a default value with `setDefaultValue()`, the setting becomes optional. You can validate a setting by giving a validator function to `setValidator()`. To get the setting from your code, use `get()`. To get whether the value returned is the optional one or the one provided, use `wasProvided()`.

For this example, if you want to provide the `percentage` setting, you will have to put in your configuration file:

```json
{
  "settings": {
    "percentage": 75
  }
}
```

See the following example for a concrete use case:

[![GitHub logo](../assets/github.png) CustomSettings.ino](https://github.com/homieiot/homie-esp8266/blob/develop/examples/CustomSettings/CustomSettings.ino)

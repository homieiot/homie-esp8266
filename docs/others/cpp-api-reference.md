# Homie

You don't have to instantiate an `Homie` instance, it is done internally.

```c++
void setup();
```

Setup Homie.

!!! warning "Mandatory!"
    Must be called once in `setup()`.

```c++
void loop();
```

Handle Homie work.

!!! warning "Mandatory!"
    Must be called once in `loop()`.

## Functions to call *before* `Homie.setup()`

```c++
void Homie_setFirmware(const char* name, const char* version);
// This is not a typo
```

Set the name and version of the firmware.
This is useful for OTA, as Homie will check against the server if there is a newer version.
Be aware, that the function is implemented as a `define` macro.
If you want to define the name or version outside the function call, you need to do so in the form of a `define` as well. 

!!! warning "Mandatory!"
    You need to set the firmware for your sketch to work.


* **`name`**: Name of the firmware. Default value is `undefined`
* **`version`**: Version of the firmware. Default value is `undefined`

```c++
void Homie_setBrand(const char* name);
// This is not a typo
```

Set the brand of the device, used in the configuration AP, the device hostname and the MQTT client ID.

* **`name`**: Name of the brand. Default value is `Homie`

```c++
Homie& disableLogging();
```

Disable Homie logging.

```c++
Homie& setLoggingPrinter(Print* printer);
```

Set the Print instance used for logging.

* **`printer`**: Print instance to log to. By default, `Serial` is used

!!! warning
    It's up to you to call `Serial.begin()`

```c++
Homie& disableLedFeedback();
```

Disable the built-in LED feedback indicating the Homie for ESP8266 state.

```c++
Homie& setLedPin(uint8_t pin, uint8_t on);
```

Set pin of the LED to control.

* **`pin`**: LED to control
* **`on`**: state when the light is on (HIGH or LOW)

```c++
Homie& setConfigurationApPassword(const char* password);
```

Set the configuration AP password.

* **`password`**: the configuration AP password

```c++
Homie& setGlobalInputHandler(std::function<bool(const String& nodeId, const String& property, const HomieRange& range, const String& value)> handler);
```

Set input handler for subscribed properties.

* **`handler`**: Global input handler
* **`node`**: Name of the node getting updated
* **`property`**: Property of the node getting updated
* **`range`**: Range of the property of the node getting updated
* **`value`**: Value of the new property

```c++
Homie& setBroadcastHandler(std::function<bool(const String& level, const String& value)> handler);
```

Set broadcast handler.

* **`handler`**: Broadcast handler
* **`level`**: Level of the broadcast
* **`value`**: Value of the broadcast

```c++
Homie& onEvent(std::function<void(const HomieEvent& event)> callback);
```

Set the event handler. Useful if you want to hook to Homie events.

* **`callback`**: Event handler

```c++
Homie& setResetTrigger(uint8_t pin, uint8_t state, uint16_t time);
```

Set the reset trigger. By default, the device will reset when pin `0` is `LOW` for `5000`ms.

* **`pin`**: Pin of the reset trigger
* **`state`**: Reset when the pin reaches this state for the given time
* **`time`**: Time necessary to reset

```c++
Homie& disableResetTrigger();
```

Disable the reset trigger.

```c++
Homie& setSetupFunction(std::function<void()> callback);
```

You can provide the function that will be called when operating in `normal` mode.

* **`callback`**: Setup function

```c++
Homie& setLoopFunction(std::function<void()> callback);
```

You can provide the function that will be looped in normal mode.

* **`callback`**: Loop function

```c++
Homie& setStandalone();
```

This will mark the Homie firmware as standalone, meaning it will first boot in `standalone` mode. To configure it and boot to `configuration` mode, the device has to be resetted.

## Functions to call *after* `Homie.setup()`

```c++
void reset();
```

Flag the device for reset.

```c++
void setIdle(bool idle);
```

Set the device as idle or not. This is useful at runtime, because you might want the device not to be resettable when you have another library that is doing some unfinished work, like moving shutters for example.

* **`idle`**: Device in an idle state or not

```c++
void prepareToSleep();
```

Prepare the device for deep sleep. It ensures messages are sent and disconnects cleanly from the MQTT broker, triggering a `READY_TO_SLEEP` event when done.

```c++
void doDeepSleep(uint32_t time_us = 0, RFMode mode = RF_DEFAULT);
```

Puth the device into deep sleep. It ensures the Serial is flushed.

```c++
bool isConfigured() const;
```

Is the device in `normal` mode, configured?

```c++
bool isConnected() const;
```

Is the device in `normal` mode, configured and connected?

```c++
const ConfigStruct& getConfiguration() const;
```

Get the configuration struct.

!!! danger
    Be careful with this struct, never attempt to change it.

```c++
AsyncMqttClient& getMqttClient();
```

Get the underlying `AsyncMqttClient` object.

```c++
Logger& getLogger();
```

Get the underlying `Logger` object, which is only a wrapper around `Serial` by default.

-------

# HomieNode

```c++
HomieNode(const char* id, const char* type, std::function<bool(const String& property, const HomieRange& range, const String& value)> handler = );
```

Constructor of an HomieNode object.

* **`id`**: ID of the node
* **`type`**: Type of the node
* **`handler`**: Optional. Input handler of the node

```c++
const char* getId() const;
```

Return the ID of the node.

```c++
const char* getType() const;
```

Return the type of the node.

```c++
PropertyInterface& advertise(const char* property);
PropertyInterface& advertiseRange(const char* property, uint16_t lower, uint16_t upper);
```

Advertise a property / range property on the node.

* **`property`**: Property to advertise
* **`lower`**: Lower bound of the range
* **`upper`**: Upper bound of the range

This returns a reference to `PropertyInterface` on which you can call:

```c++
void settable(std::function<bool(const HomieRange& range, const String& value)> handler) = );
```

Make the property settable.

* **`handler`**: Optional. Input handler of the property

```c++
SendingPromise& setProperty(const String& property);
```

Using this function, you can set the value of a node property, like a temperature for example.

* **`property`**: Property to send

This returns a reference to `SendingPromise`, on which you can call:

```c++
SendingPromise& setQos(uint8_t qos);  // defaults to 1
SendingPromise& setRetained(bool retained);  // defaults to true
SendingPromise& overwriteSetter(bool overwrite);  // defaults to false
SendingPromise& setRange(const HomieRange& range);  // defaults to not a range
SendingPromise& setRange(uint16_t rangeIndex);  // defaults to not a range
uint16_t send(const String& value);  // finally send the property, return the packetId (or 0 if failure)
```

Method names should be self-explanatory.

# HomieSetting

```c++
HomieSetting<T>(const char* name, const char* description);
```

Constructor of an HomieSetting object.

* **`T`**: Type of the setting. Either `bool`, `unsigned long`, `long`, `double` or `const char*`
* **`name`**: Name of the setting
* **`description`**: Description of the setting

```c++
T get() const;
```

Get the default value if the setting is optional and not provided, or the provided value if the setting is required or optional but provided.

```c++
bool wasProvided() const;
```

Return whether the setting was provided or not (otherwise `get()` would return the default value).

Set the default value and make the setting optional.

```c++
HomieSetting<T>& setDefaultValue(T defaultValue);
```

* **`defaultValue`**: The default value

```c++
HomieSetting<T>& setValidator(std::function<bool(T candidate)> validator);
```

Set a validation function for the setting. The validator must return `true` if the candidate is correct, `false` otherwise.

* **`validator`**: The validation function

# Know if the device is configured / connected

If, for some reason, you want to run some code in the Arduino `loop()` function, it might be useful for you to know if the device is in configured (so in `normal` mode) and if the network connection is up.

```c++
void loop() {
  if (Homie.isConfigured()) {
    // The device is configured, in normal mode
    if (Homie.isConnected()) {
      // The device is connected
    } else {
      // The device is not connected
    }
  } else {
    // The device is not configured, in either configuration or standalone mode
  }
}
```

# Get access to the configuration

You can get access to the configuration of the device. The representation of the configuration is:

```c++
struct ConfigStruct {
  char* name;
  char* deviceId;

  struct WiFi {
    char* ssid;
    char* password;
  } wifi;

  struct MQTT {
    struct Server {
      char* host;
      uint16_t port;
    } server;
    char* baseTopic;
    bool auth;
    char* username;
    char* password;
  } mqtt;

  struct OTA {
    bool enabled;
  } ota;
};
```

For example, to access the Wi-Fi SSID, you would do:

```c++
Homie.getConfiguration().wifi.ssid;
```

# Get access to the MQTT client

You can get access to the underlying MQTT client. For example, to disconnect from the broker:

```c++
Homie.getMqttClient().disconnect();
```

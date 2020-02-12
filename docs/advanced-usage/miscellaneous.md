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

# Organizing the sketch in good and safe order

The example above is good to demonstrate usage of `Homie.isConnected()`. However, consider do not adding any code into `loop()` function. ESP8266 is relatively weak MCU, and Homie manages few important environmental tasks. Interferring with Homie might boomerang by sudden crashes. Advised sketch of Homie is:
```c++
#include <Homie.h>

void setupHandler() {
  // Code which should run AFTER the WiFi is connected.
}

void loopHandler() {
  // Code which should run in normal loop(), after setupHandler() finished.
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("bare-minimum", "1.0.0"); // The underscore is not a typo! See Magic bytes
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  
 // Code which should run BEFORE the WiFi is connected.

  Homie.setup();
}

void loop() {
  Homie.loop();
}
```

This way you can be sure the run is safe enough (unless you use blocking delay, then it is not) and the sketch keeps 'regular' structure of `setup()`/`loop()` just in terms of Homie.

## Why it's needed `setupHandler()` in addition to `setup()` in Homie?
`setup()` starts execution immediately after power on/wake up. `setupHandler()` starts to run when WiFi established the connection.

## Why it's needed `loopHandler()` in addition to `loop()` in Homie?
`loop()` starts to run then `setup() is completed`. This somehow controls a run of `setupHandler()`. The `loopHandler()` serves safe way to run the loop.

## Why this understanding is important?
1. Once your code in `setupHandler()` and `loopHandler()` engaged, you can be pretty sure the `Homie.isConnected()` is true without checking it. Unless, you intend your device is mobile and might reach out of WiFi coverage in the middle of the run.
1. The `loop()` often starts before Wifi is connected. "Wifi connected" event causes significant load on the MCU with Wifi related tasks, also with sending initial MQTT reports. Therefore, running complex commands around the moment of "Wifi connected" might cause malfunction/crash.

As solution, heavy commands (massive initializations, long calculations, SPIFFS reading/writing, etc.) should be initiated in one of 2 cases:
 - very early in setup() before "Wifi connected".
 - way after "Wifi connected" (non-blocking wait 3-5 seconds and then do the heavy commands).

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

# Keep and get MQTT reports while WiFi is not connected

```c++
#include <cppQueue.h>

HomieNode myNode("q_test", "test"); 

// Struct for Queueing of postponed MQTT messages. Needed to gather messages while Wifi isn't connected yet.
typedef struct strRec {
  char topic[10];
  char msg[90];
} Rec;
Queue msg_q(sizeof(Rec), 7, FIFO, true);  // RAM is limited. Keep reasonable Q length (5-10 messages).

// Here comes all other stuff, inclusing setup() and loop().

```

In functions, where WiFi is not expected, keep the report as example:
```c++
Rec r = {"alert", "Low storage space is free on SPIFF."};
msg_q.push(&r);
```

To release (AKA send) all kept messages from the queue, add in `loopHandler()`:
```c++
while (!msg_q.isEmpty() && Homie.isConnected()) {
  Rec r;
  msg_q.pop(&r);
  myNode.setProperty(r.topic).setRetained(false).send(r.msg);
}
```

It might be good practice to use the queue always, not only when WiFi is down. This might make Homie job easier, and you get stability as benefit.

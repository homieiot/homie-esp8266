You may want to hook to Homie events. Maybe you will want to control an RGB LED if the Wi-Fi connection is lost, or execute some code prior to a device reset, for example to clear some EEPROM you're using:

```c++
void onHomieEvent(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::STANDALONE_MODE:
      // Do whatever you want when standalone mode is started
      break;
    case HomieEventType::CONFIGURATION_MODE:
      // Do whatever you want when configuration mode is started
      break;
    case HomieEventType::NORMAL_MODE:
      // Do whatever you want when normal mode is started
      break;
    case HomieEventType::OTA_STARTED:
      // Do whatever you want when OTA is started
      break;
    case HomieEventType::OTA_PROGRESS:
      // Do whatever you want when OTA is in progress

      // You can use event.sizeDone and event.sizeTotal
      break;
    case HomieEventType::OTA_FAILED:
      // Do whatever you want when OTA is failed
      break;
    case HomieEventType::OTA_SUCCESSFUL:
      // Do whatever you want when OTA is successful
      break;
    case HomieEventType::ABOUT_TO_RESET:
      // Do whatever you want when the device is about to reset
      break;
    case HomieEventType::WIFI_CONNECTED:
      // Do whatever you want when Wi-Fi is connected in normal mode

      // You can use event.ip, event.gateway, event.mask
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      // Do whatever you want when Wi-Fi is disconnected in normal mode

      // You can use event.wifiReason
      /*
        Wi-Fi Reason (souce: https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEvents/WiFiClientEvents.ino)
        0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
        1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
        2  SYSTEM_EVENT_STA_START                < ESP32 station start
        3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
        4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
        5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
        6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
        7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
        8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
        9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
        10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
        11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
        12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
        13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
        14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
        15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
        16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
        17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
        18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
        19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
        20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
        21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
        22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
        23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
        24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
        25 SYSTEM_EVENT_MAX
      */
      break;
    case HomieEventType::MQTT_READY:
      // Do whatever you want when MQTT is connected in normal mode
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      // Do whatever you want when MQTT is disconnected in normal mode

      // You can use event.mqttReason
      /*
        MQTT Reason (source: https://github.com/marvinroger/async-mqtt-client/blob/master/src/AsyncMqttClient/DisconnectReasons.hpp)
        0 TCP_DISCONNECTED
        1 MQTT_UNACCEPTABLE_PROTOCOL_VERSION
        2 MQTT_IDENTIFIER_REJECTED
        3 MQTT_SERVER_UNAVAILABLE
        4 MQTT_MALFORMED_CREDENTIALS
        5 MQTT_NOT_AUTHORIZED
        6 ESP8266_NOT_ENOUGH_SPACE
        7 TLS_BAD_FINGERPRINT
      */
      break;
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      // Do whatever you want when an MQTT packet with QoS > 0 is acknowledged by the broker

      // You can use event.packetId
      break;
    case HomieEventType::READY_TO_SLEEP:
      // After you've called `prepareToSleep()`, the event is triggered when MQTT is disconnected
      break;
    case HomieEventType::SENDING_STATISTICS:
      // Do whatever you want when statistics are sent in normal mode
      break;
  }
}

void setup() {
  Homie.onEvent(onHomieEvent); // before Homie.setup()
  // ...
}
```

See the following example for a concrete use case:

[![GitHub logo](../assets/github.png) HookToEvents.ino](https://github.com/homieiot/homie-esp8266/blob/develop/examples/HookToEvents/HookToEvents.ino)

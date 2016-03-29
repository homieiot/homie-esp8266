#ifndef HomieEvent_h
#define HomieEvent_h

enum HomieEvent : unsigned char {
    HOMIE_CONFIGURATION_MODE = 1,
    HOMIE_NORMAL_MODE = 2,
    HOMIE_OTA_MODE = 3,
    HOMIE_ABOUT_TO_RESET = 4,
    HOMIE_WIFI_CONNECTED = 5,
    HOMIE_WIFI_DISCONNECTED = 6,
    HOMIE_MQTT_CONNECTED = 7,
    HOMIE_MQTT_DISCONNECTED = 8
};

#endif

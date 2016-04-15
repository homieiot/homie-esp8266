#include <Homie.h>

void onHomieEvent(HomieEvent event) {
  switch(event) {
    case HOMIE_CONFIGURATION_MODE:
      Serial.println("Configuration mode started");
      break;
    case HOMIE_NORMAL_MODE:
      Serial.println("Normal mode started");
      break;
    case HOMIE_OTA_MODE:
      Serial.println("OTA mode started");
      break;
    case HOMIE_ABOUT_TO_RESET:
      Serial.println("About to reset");
      break;
    case HOMIE_WIFI_CONNECTED:
      Serial.println("Wi-Fi connected");
      break;
    case HOMIE_WIFI_DISCONNECTED:
      Serial.println("Wi-Fi disconnected");
      break;
    case HOMIE_MQTT_CONNECTED:
      Serial.println("MQTT connected");
      break;
    case HOMIE_MQTT_DISCONNECTED:
      Serial.println("MQTT disconnected");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Homie.enableLogging(false);
  Homie.setFirmware("events-test", "1.0.0");
  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

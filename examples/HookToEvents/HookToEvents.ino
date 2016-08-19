#include <Homie.h>

void onHomieEvent(HomieEvent event) {
  switch(event) {
    case HomieEvent::CONFIGURATION_MODE:
      Serial.println("Configuration mode started");
      break;
    case HomieEvent::NORMAL_MODE:
      Serial.println("Normal mode started");
      break;
    case HomieEvent::OTA_STARTED:
      Serial.println("OTA started");
      break;
    case HomieEvent::OTA_FAILED:
      Serial.println("OTA failed");
      break;
    case HomieEvent::OTA_SUCCESSFUL:
      Serial.println("OTA successful");
      break;
    case HomieEvent::ABOUT_TO_RESET:
      Serial.println("About to reset");
      break;
    case HomieEvent::WIFI_CONNECTED:
      Serial.println("Wi-Fi connected");
      break;
    case HomieEvent::WIFI_DISCONNECTED:
      Serial.println("Wi-Fi disconnected");
      break;
    case HomieEvent::MQTT_CONNECTED:
      Serial.println("MQTT connected");
      break;
    case HomieEvent::MQTT_DISCONNECTED:
      Serial.println("MQTT disconnected");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Homie.enableLogging(false);
  Homie_setFirmware("events-test", "1.0.0");
  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

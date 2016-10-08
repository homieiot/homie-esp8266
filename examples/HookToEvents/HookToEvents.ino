#include <Homie.h>

void onHomieEvent(const HomieEvent& event) {
  switch (event.type) {
    case HomieEventType::STANDALONE_MODE:
      Serial.println("Standalone mode started");
      break;
    case HomieEventType::CONFIGURATION_MODE:
      Serial.println("Configuration mode started");
      break;
    case HomieEventType::NORMAL_MODE:
      Serial.println("Normal mode started");
      break;
    case HomieEventType::OTA_STARTED:
      Serial.println("OTA started");
      break;
    case HomieEventType::OTA_FAILED:
      Serial.println("OTA failed");
      break;
    case HomieEventType::OTA_SUCCESSFUL:
      Serial.println("OTA successful");
      break;
    case HomieEventType::ABOUT_TO_RESET:
      Serial.println("About to reset");
      break;
    case HomieEventType::WIFI_CONNECTED:
      Serial.print("Wi-Fi connected, IP: ");
      Serial.print(event.ip);
      Serial.print(", gateway: ");
      Serial.print(event.gateway);
      Serial.print(", mask: ");
      Serial.println(event.mask);
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      Serial.print("Wi-Fi disconnected, reason: ");
      Serial.println((int8_t)event.wifiReason);
      break;
    case HomieEventType::MQTT_CONNECTED:
      Serial.println("MQTT connected");
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      Serial.print("MQTT disconnected, reason: ");
      Serial.println((int8_t)event.mqttReason);
      break;
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      Serial.print("MQTT packet acknowledged, packetId: ");
      Serial.println(event.packetId);
      break;
    case HomieEventType::READY_TO_SLEEP:
      Serial.println("Ready to sleep");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Homie.disableLogging();
  Homie_setFirmware("events-test", "1.0.0");
  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

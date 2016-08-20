#include <Homie.h>

const int TEMPERATURE_INTERVAL = 300;

unsigned long lastTemperatureSent = 0;

HomieNode temperatureNode("temperature", "temperature");

void setupHandler() {
  Homie.setNodeProperty(temperatureNode, "unit", "c", true);
}

void loopHandler() {
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    float temperature = 22; // Fake temperature here, for the example
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Homie.setNodeProperty(temperatureNode, "degrees", String(temperature));
    lastTemperatureSent = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Homie_setFirmware("awesome-temperature", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  temperatureNode.advertise("unit");
  temperatureNode.advertise("degrees");

  Homie.setup();
}

void loop() {
  Homie.loop();
}

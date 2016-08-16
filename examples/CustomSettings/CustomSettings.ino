#include <Homie.h>

const int DEFAULT_TEMPERATURE_INTERVAL = 300;

unsigned long lastTemperatureSent = 0;

HomieNode temperatureNode("temperature", "temperature");

HomieSetting<unsigned long> temperatureIntervalSetting("temperatureInterval", "The temperature interval in milliseconds");

void setupHandler() {
  Homie.setNodeProperty(temperatureNode, "unit", "c", true);
}

void loopHandler() {
  if (millis() - lastTemperatureSent >= temperatureIntervalSetting.get() || lastTemperatureSent == 0) {
    float temperature = 22; // Fake temperature here, for the example
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Homie.setNodeProperty(temperatureNode, "degrees", String(temperature), true);
    lastTemperatureSent = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Homie_setFirmware("temperature-setting", "1.0.0");
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);

  temperatureIntervalSetting.setDefaultValue(DEFAULT_TEMPERATURE_INTERVAL);

  Homie.setup();
}

void loop() {
  Homie.loop();
}

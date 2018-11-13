#include <Homie.h>

const int DEFAULT_TEMPERATURE_INTERVAL = 300;

unsigned long lastTemperatureSent = 0;

HomieNode temperatureNode("temperature", "Temperature", "temperature");

HomieSetting<long> temperatureIntervalSetting("temperatureInterval", "The temperature interval in seconds");

void loopHandler() {
  if (millis() - lastTemperatureSent >= temperatureIntervalSetting.get() * 1000UL || lastTemperatureSent == 0) {
    float temperature = 22; // Fake temperature here, for the example
    Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
    temperatureNode.setProperty("degrees").send(String(temperature));
    lastTemperatureSent = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("temperature-setting", "1.0.0");
  Homie.setLoopFunction(loopHandler);

  temperatureNode.advertise("degrees").setName("Degrees")
                                      .setDatatype("float")
                                      .setUnit("ºC");

  temperatureIntervalSetting.setDefaultValue(DEFAULT_TEMPERATURE_INTERVAL).setValidator([] (long candidate) {
    return candidate > 0;
  });

  Homie.setup();
}

void loop() {
  Homie.loop();
}

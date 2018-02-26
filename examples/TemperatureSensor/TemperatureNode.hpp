#include <Homie.h>
#include <Wire.h>
#include "ClosedCube_SHT31D.h"
#include "Nodes/Node.hpp"
namespace Node
{
ClosedCube_SHT31D sht3xd;
const int TEMPERATURE_INTERVAL = 10;
unsigned long lastTemperatureSent = 0;
class TemperatureNode : public Node
{
  private:
    SHT31D mSensor;
    using String = std::string;

  public:
    explicit TemperatureNode(const std::string &str) : Node(str) {}
    void setup() override
    {
        Wire.begin();
        sht3xd.begin(0x44); // I2C address: 0x44 or 0x45
        Serial.print("Serial #");
        Serial.println(sht3xd.readSerialNumber());
    }
    void loop() override
    {
        if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0)
        {
            mSensor = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
            Homie.getLogger() << "(Temperature °C,Humidity %): " << toString() << endl;
            HomieInternals::Interface::get()
                .getSendingPromise()
                .send(getPath(), toString());
            lastTemperatureSent = millis();
        }
    }
    int handleMsg(const std::string &node_name, const Binary &data) override { return 0; }
    String toString() override
    {
        return toString(mSensor.t) + "," + toString(mSensor.rh);
    }
    String toString(float f)
    {
        char tmp[10];
        sprintf(tmp, "%.2f", f);
        return String(tmp);
    }
};
}

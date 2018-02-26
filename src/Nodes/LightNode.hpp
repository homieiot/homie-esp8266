#pragma once
#include <Arduino.h>
#include "Node.hpp"
namespace Node
{
class LightNode : public Node
{
  private:
    bool isOn = false;
    uint8_t pin;

  public:
    explicit LightNode(uint8_t p, const String &str) : Node(str)
    {
        pin = p;
    }
    void setup() override
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, isOn ? LOW : HIGH);
    }
    int handleMsg(const String &node_name, const Binary &data) override
    {
        return HandleError::not_find;
    }
    int handleMsg(const Binary &data) override
    {
        isOn = data[0] == '1';
        digitalWrite(pin, isOn ? LOW : HIGH);
        return HandleError::success;
    }
    String toString() override
    {
        return isOn ? "On" : "Off";
    }
};
}

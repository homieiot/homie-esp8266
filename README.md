![homie-esp8266 banner](banner.png)

Enhance [Homie](https://github.com/marvinroger/homie-esp8266)
=================
Homie has many good features. This project is based of it.Our home will have too many devices too control them. My aim is finding a way to control them.

In fact many devices have some same parts. Most devices have a led light to show status.If we add more led lights to show more messages, we may change our protocol or program. Can we create a new device by one light add one light?

This project's aim is resolving it.

## First define a light node
```c++
#pragma once
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
```

## Second create an instance  and regist it to master
```
Node::LightNode l1(LED_BUILTIN, "l1");
HomieNode::getMaster()->addNode(&l1);
```
Now send mqttmsg topic("basetopic/device_id/$set/l1"),payload(1) to control.

## Third create a nested node contain two light
```
Node::NestedNode light2("light2");
Node::LightNode l1(LED_BUILTIN, "l1");
Node::LightNode l2(D6, "l2");

light2.addNode(&l1);
light2.addNode(&l2);
HomieNode::getMaster()->addNode(&light2);
```

Now send mqttmsg topic("basetopic/device_id/$set/light2/l1"),payload(1) to control light1.

topic("basetopic/device_id/$set/light2/l2"),payload(1) to control light2.

Or topic("basetopic/device_id/$set/light2") payload({"l1":1,"l2":1}) to control both!.

# Features
1. create nested node.
2. use node_path to control one node.
3. send json msg to control multi-node.

# Difference between Homie
I rewrite the part of HomieNode, and make few changes in other place. And now it will not send properties. But can still use SendPromise.

Homie subscribes `base/device_id/+/+/set` to control, and now subscribes `base/device_id/$set`. 

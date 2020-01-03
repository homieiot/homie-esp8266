In all the previous examples you have seen, node properties were advertised one-by-one (e.g. `temperature`, `unit`...). But what if you have a LED strip with, say, 100 properties, one for each LED? You won't advertise these 100 LEDs one-by-one. This is what range properties are meant for.

```c++
HomieNode stripNode("strip", "strip");

bool ledHandler(const HomieRange& range, const String& value) {
  Homie.getLogger() << "LED " << range.index << " set to " << value << endl;

  // Now, let's update the actual state of the given led
  stripNode.setProperty("led").setRange(range).send(value);
}

void setup() {
  stripNode.advertiseRange("led", 1, 100).settable(ledHandler);
  // before Homie.setup()
}
```

On the mqtt broker you will see the following message show up:
```
topic                               message
--------------------------------------------------------
homie/<device id>/strip/$type	      strip
homie/<device id>/strip/$properties	led[1-100]:settable
```

You can then publish the value `on` to topic `homie/<device id>/strip/led_1/set` to turn on led number 1.

See the following example for a concrete use case:

[![GitHub logo](../assets/github.png) LedStrip](https://github.com/homieiot/homie-esp8266/blob/develop/examples/LedStrip/LedStrip.ino)

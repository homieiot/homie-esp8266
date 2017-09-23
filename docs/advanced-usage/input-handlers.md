There are four types of input handlers:

* Global input handler. This unique handler will handle every changed settable properties for all nodes

```c++
bool globalInputHandler(const HomieNode& node, const String& property, const HomieRange& range, const String& value) {

}

void setup() {
  Homie.setGlobalInputHandler(globalInputHandler); // before Homie.setup()
  // ...
}
```

* Node input handlers. This handler will handle every changed settable properties of a specific node

```c++
bool nodeInputHandler(const String& property, const HomieRange& range, const String& value) {

}

HomieNode node("id", "type", nodeInputHandler);
```

* Virtual callback from node input handler

You can create your own class derived from HomieNode that implements the virtual method `bool HomieNode::handleInput(const String& property, const String& value)`. The default node input handler then automatically calls your callback.

```c++
class RelaisNode : public HomieNode {
 public:
	RelaisNode(): HomieNode("Relais", "switch8");

 protected:
  virtual bool handleInput(const String& property, const HomieRange& range, const String& value) {

  }
};
```

* Property input handlers. This handler will handle changes for a specific settable property of a specific node

```c++
bool propertyInputHandler(const HomieRange& range, const String& value) {

}

HomieNode node("id", "type");

void setup() {
  node.advertise("property").settable(propertyInputHandler); // before Homie.setup()
  // ...
}
```

You can see that input handlers return a boolean. An input handler can decide whether or not it handled the message and want to propagate it down to other input handlers. If an input handler returns `true`, the propagation is stopped, if it returns `false`, the propagation continues. The order of propagation is global handler → node handler → property handler.

For example, imagine you defined three input handlers: the global one, the node one, and the property one. If the global input handler returns `false`, the node input handler will be called. If the node input handler returns `true`, the propagation is stopped and the property input handler won't be called. You can think of it as middlewares.


!!! warning
    Homie uses [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) for network communication that make uses of asynchronous callback from the ESP8266 framework for incoming network packets. Thus the input handler runs in a different task than the `loopHandler()`. So keep in mind that the network task may interrupt your loop at any time.

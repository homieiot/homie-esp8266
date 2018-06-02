Your device can react to Homie broadcasts. To do that, you can use a broadcast handler:

```c++
bool broadcastHandler(const String& level, const String& value) {
  Serial << "Received broadcast level " << level << ": " << value << endl;
  return true;
}

void setup() {
  Homie.setBroadcastHandler(broadcastHandler); // before Homie.setup()
  // ...
}
```

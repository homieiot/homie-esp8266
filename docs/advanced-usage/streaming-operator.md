Homie for ESP8266 includes a nice streaming operator to interact with `Print` objects.

Imagine the following code:

```c++
int temperature = 32;
Homie.getLogger().print("The current temperature is ");
Homie.getLogger().print(temperature);
Homie.getLogger().println(" °C.");
```

With the streaming operator, the following code will do exactly the same thing, without performance penalties:

```c++
int temperature = 32;
Homie.getLogger() << "The current temperature is " << temperature << " °C." << endl;
```

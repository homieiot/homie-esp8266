# SSL support

In Homie for ESP8266 v1.x, SSL was possible but it was not reliable. Due to the asynchronous nature of the v2.x, SSL is not completely available anymore. Only MQTT connections can be encrypted with SSL.

# ADC readings

[This is a known esp8266/Arduino issue](https://github.com/esp8266/Arduino/issues/1634) that polling `analogRead()` too frequently forces the Wi-Fi to disconnect. As a workaround, don't poll the ADC more than one time every 3ms.

# Wi-Fi connection

If you encouter any issues with the Wi-Fi, try changing the flash size build parameter, or try to erase the flash. See [#158](https://github.com/homieiot/homie-esp8266/issues/158) for more information.

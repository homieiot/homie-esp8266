`data/homie` folder
===================

This folder contains the data you can upload to the SPIFFS of your ESP8266.
This is optional.

To upload files to the SPIFFS of your device, create a folder named `data` in your sketch directory. In this `data` folder, create an `homie` directory. You can put two files in it:

1. The `config.json` file, if you want to bypass the `configuration` mode.
2. The `ui_bundle.gz` file, that you can download [here](http://setup.homie-esp8266.marvinroger.fr/ui_bundle.gz). If present, the configuration UI will be served directly from the ESP8266.

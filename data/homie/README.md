`data/homie` folder
===================

This folder contains the data you can upload to the SPIFFS of your ESP8266.
This is optional.

To upload files to the SPIFFS of your device, first create a folder named `data` in your sketch directory. In this `data` folder, create an `homie` directory. You can put two files in it:

1. The `config.json` file, if you want to bypass the `configuration` mode.
2. The `ui_bundle.gz` file, that you can download [here](http://setup.homie-esp8266.marvinroger.fr/ui_bundle.gz). If present, the configuration UI will be served directly from the ESP8266.

Finally initiate the [SPIFFS upload process](http://docs.platformio.org/en/stable/platforms/espressif8266.html?highlight=spiffs#uploading-files-to-file-system-spiffs) via PlatformIO, or via the [Arduino IDE](http://esp8266.github.io/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system)

Configurator for Homie for ESP8266
==================================

UI to configure an ESP8266 loaded with an Homie firmware.

## Installation

### Hosted version

Open http://marvinroger.github.io/homie-esp8266/ in your web browser.

### Locally

[Download the content of this branch](https://github.com/marvinroger/homie-esp8266/archive/gh-pages.zip) and open `index.html` in your web browser.

## Usage

In `config` mode, your Homie device will generate a Wi-FI AP named `Homie-XXXXXXXX`. The password is the id (`XXXXXXXX`). For example, if the AP is named `Homie-00a2e44d`, the password is `00a2e44d`. Connect to it and follow the instructions that should show on the web page.

## Contribute

Contributions are very welcome!

To build assets, just run `npm run dev`.
This will build the public directory, and watch for changes in the `app` folder.

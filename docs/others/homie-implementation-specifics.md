The Homie `$implementation` identifier is `esp8266`.

# Version

* `$implementation/version`: Homie for ESP8266 version

# Reset

* `$implementation/reset`: You can publish a `true` to this topic to reset the device

# Configuration

* `$implementation/config`: The `configuration.json` is published there, with `wifi.password`, `mqtt.username` and `mqtt.password` fields stripped
* `$implementation/config/set`: You can update the `configuration.json` by sending incremental JSON on this topic

# OTA

* `$implementation/ota/enabled`: `true` if OTA is enabled, `false` otherwise
* `$implementation/ota/firmware`: If the update request is accepted, you must send the firmware payload to this topic
* `$implementation/ota/status`: HTTP-like status code indicating the status of the OTA. Might be:

Code|Description
----|-----------
`200`|OTA successfully flashed
`202`|OTA request / checksum accepted
`206 465/349680`|OTA in progress. The data after the status code corresponds to `<bytes written>/<bytes total>`
`304`|The current firmware is already up-to-date
`400 BAD_FIRMWARE`|OTA error from your side. The identifier might be `BAD_FIRMWARE`, `BAD_CHECKSUM`, `NOT_ENOUGH_SPACE`, `NOT_REQUESTED`
`403`|OTA not enabled
`500 FLASH_ERROR`|OTA error on the ESP8266. The identifier might be `FLASH_ERROR`

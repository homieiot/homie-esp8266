To configure your device, you have two choices: manually flashing the configuration file to the SPIFFS at the `/homie/config.json` (see [Uploading files to file system](http://esp8266.github.io/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system)), so you can bypass the `configuration` mode, or send it through the [HTTP JSON API](http-json-api.md).

Below is the format of the JSON configuration you will have to provide:

```json
{
  "name": "The kitchen light",
  "device_id": "kitchen-light",
  "device_stats_interval": 60,
  "wifi": {
    "ssid": "Network_1",
    "password": "I'm a Wi-Fi password!",
    "bssid": "DE:AD:BE:EF:BA:BE",
    "channel": 1,
    "ip": "192.168.1.5",
    "mask": "255.255.255.0",
    "gw": "192.168.1.1",
    "dns1": "8.8.8.8",
    "dns2": "8.8.4.4"
  },
  "mqtt": {
    "host": "192.168.1.10",
    "port": 1883,
    "base_topic": "devices/",
    "auth": true,
    "username": "user",
    "password": "pass",
    "ssl": true,
    "ssl_fingerprint": "a27992d3420c89f293d351378ba5f5675f74fe3c"
  },
  "ota": {
    "enabled": true
  },
  "settings": {
    "percentage": 55
  }
}
```

The above JSON contains every field that can be customized.

Here are the rules:

* `name`, `wifi.ssid`, `wifi.password`, `mqtt.host` and `ota.enabled` are mandatory
* `wifi.password` can be `null` if connecting to an open network
* If `mqtt.auth` is `true`, `mqtt.username` and `mqtt.password` must be provided
* `bssid`, `channel`, `ip`, `mask`, `gw`, `dns1`, `dns2` are not mandatory and are only needed to if there is a requirement to specify particular AP or set Static IP address. There are some rules which needs to be satisfied:
   - `bssid` and `channel` have to be defined together and these settings are independand of settings related to static IP
   - to define static IP, `ip` (IP address), `mask` (netmask) and `gw` (gateway) settings have to be defined at the same time
   - to define second DNS `dns2` the first one `dns1` has to be defined. Set DNS without `ip`, `mask` and `gw` does not affect the configuration (dns server will be provided by DHCP). It is not required to set DNS servers.
* `ssl_fingerprint` can optionally be defined if `ssl` is enabled. The public key of the MQTT server is then verified against the fingerprint.

Default values if not provided:

* `device_id`: the hardware device ID (eg. `1a2b3c4d5e6f`)
* `device_stats_interval`: 60 seconds
* `mqtt.port`: `1883`
* `mqtt.base_topic`: `homie/`
* `mqtt.auth`: `false`

The `mqtt.host` field can be either an IP or an hostname.

!!! tip "OTA"
    Homie-esp8266 supports `Over the Air` update if you enabled this in configuration (`ota.enabled: true`).
    For more details see: [OTA updates](../others/ota-configuration-updates.md)

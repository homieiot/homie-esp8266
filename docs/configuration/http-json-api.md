When in `configuration` mode, the device exposes a HTTP JSON API to send the configuration to it. When you send a valid configuration to the `/config` endpoint, the configuration file is stored in the filesystem at `/homie/config.json`.

If you don't want to mess with JSON, you have a Web UI / app available:

* At http://homieiot.github.io/homie-esp8266/configurators/v2/
* As an [Android app](https://build.phonegap.com/apps/1906578/share)

**Quick instructions to use the Web UI / app**:

1. Open the Web UI / app
2. Disconnect from your current Wi-Fi AP, and connect to the `Homie-xxxxxxxxxxxx` AP spawned in `configuration` mode
3. Follow the instructions

You can see the sources of the Web UI [here](https://github.com/marvinroger/homie-esp8266-setup).

Alternatively, you can use this `curl` command to send the configuration to the device. You must connect to the device in `configuration` mode (i.e. the device is an Access Point). This method will not work if not in `configuration` mode:

```shell
curl -X PUT http://192.168.123.1/config --header "Content-Type: application/json" -d @config.json
```

This will send the `./config.json` file to the device.

# Error handling

When everything went fine, a `2xx` HTTP code is returned, such as `200 OK`, `202 Accepted`, `204 No Content` and so on.
If anything goes wrong, a return code != 2xx will be returned, with a JSON `error` field indicating the error, such as `500 Internal Server error`, `400 Bad request` and so on.

# Endpoints

**API base address:** `http://192.168.123.1`

??? summary "GET `/heart`"
    This is useful to ensure we are connected to the device AP.

    ## Response

    `204 No Content`

--------------

??? summary "GET `/device-info`"
    Get some information on the device.

    ## Response

    `200 OK (application/json)`

    ```json
    {
      "hardware_device_id": "52a8fa5d",
      "homie_esp8266_version": "2.0.0",
      "firmware": {
        "name": "awesome-device",
        "version": "1.0.0"
      },
      "nodes": [
        {
          "id": "light",
          "type": "light"
        }
      ],
      "settings": [
        {
          "name": "timeout",
          "description": "Timeout in seconds",
          "type": "ulong",
          "required": false,
          "default": 10
        }
      ]
    }
    ```

    `type` can be one of the following:

    * `bool`: a boolean
    * `ulong`: an unsigned long
    * `long`: a long
    * `double`: a double
    * `string`: a string

    !!! note "Note about settings"
        If a setting is not required, the `default` field will always be set.

--------------

??? summary "GET `/networks`"
    Retrieve the Wi-Fi networks the device can see.

    ## Response

    !!! success "In case of success"
        `200 OK (application/json)`

        ```json
        {
          "networks": [
            { "ssid": "Network_2", "rssi": -82, "encryption": "wep" },
            { "ssid": "Network_1", "rssi": -57, "encryption": "wpa" },
            { "ssid": "Network_3", "rssi": -65, "encryption": "wpa2" },
            { "ssid": "Network_5", "rssi": -94, "encryption": "none" },
            { "ssid": "Network_4", "rssi": -89, "encryption": "auto" }
          ]
        }
        ```

    !!! failure "In case the initial Wi-Fi scan is not finished on the device"
        `503 Service Unavailable (application/json)`

        ```json
        {
          "error": "Initial Wi-Fi scan not finished yet"
        }
        ```

--------------

??? summary "PUT `/config`"
    Save the config to the device.

    ## Request body

    `(application/json)`

    See [JSON configuration file](json-configuration-file.md).

    ## Response

    !!! success "In case of success"
        `200 OK (application/json)`

        ```json
        {
          "success": true
        }
        ```

    !!! failure "In case of error in the payload"
        `400 Bad Request (application/json)`

        ```json
        {
          "success": false,
          "error": "Reason why the payload is invalid"
        }
        ```

    !!! failure "In case the device already received a valid configuration and is waiting for reboot"
        `403 Forbidden (application/json)`

        ```json
        {
          "success": false,
          "error": "Device already configured"
        }
        ```

--------------

??? summary "PUT `/wifi/connect`"
    Initiates the connection of the device to the Wi-Fi network while in configuation mode. This request is not synchronous and the result (Wi-Fi connected or not) must be obtained by with `GET /wifi/status`.

    ## Request body

    `(application/json)`

    ```json
    {
      "ssid": "My_SSID",
      "password": "my-passw0rd"
    }
    ```

    ## Response

    !!! success "In case of success"
        `202 Accepted (application/json)`

        ```json
        {
          "success": true
        }
        ```

    !!! failure "In case of error in the payload"
        `400 Bad Request (application/json)`

        ```json
        {
          "success": false,
          "error": "Reason why the payload is invalid"
        }
        ```

--------------

??? summary "GET `/wifi/status`"
    Returns the current Wi-Fi connection status.

    Helpful when monitoring Wi-Fi connectivity after `PUT /wifi/connect`.

    ## Response

    `200 OK (application/json)`

    ```json
    {
      "status": "connected"
    }
    ```

    `status` might be one of the following:

    * `idle`
    * `connect_failed`
    * `connection_lost`
    * `no_ssid_available`
    * `connected` along with a `local_ip` field
    * `disconnected`

--------------

??? summary "PUT `/proxy/control`"
    Enable/disable the device to act as a transparent proxy between AP and Station networks.

    All requests that don't collide with existing API paths will be bridged to the destination according to the `Host` HTTP header. The destination host is called using the existing Wi-Fi connection (established after a `PUT /wifi/connect`) and all contents are bridged back to the connection made to the AP side.

    This feature can be used to help captive portals to perform cloud API calls during device enrollment using the ESP8266 Wi-Fi AP connection without having to patch the Homie firmware. By using the transparent proxy, all operations can be performed by the custom JavaScript running on the browser (in SPIFFS location `/data/homie/ui_bundle.gz`).

    HTTPS is not supported.

    **Important**: The HTTP requests and responses must be kept as small as possible because all contents are transported using RAM memory, which is very limited.

    ## Request body

    `(application/json)`

    ```json
    {
      "enable": true
    }
    ```

    ## Response

    ??? success "In case of success"
        `200 OK (application/json)`

        ```json
        {
          "success": true
        }
        ```

    ??? failure "In case of error in the payload"
        `400 Bad Request (application/json)`

        ```json
        {
          "success": false,
          "error": "Reason why the payload is invalid"
        }
        ```

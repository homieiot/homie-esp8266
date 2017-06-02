Script: OTA updater
===================

This will allow you to send an OTA update to your device.

## Installation

`pip install -r requirements.txt`

## Usage

```bash
python ./ota_update.py \
  --broker-host "127.0.0.1" \
  --broker-port "1883" \
  --broker-username "" \
  --broker-password "" \
  --base-topic "homie/" \
  --device-id "my-device-id" \
  ~/my_firmware.bin
```

All parameters are optional, except `--device-id` and the firmware path.
Default values are the one above.

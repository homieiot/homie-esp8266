#!/usr/bin/env python

from __future__ import division, print_function
import paho.mqtt.client as mqtt
import base64, sys, math
from hashlib import md5

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc != 0:
        print("Connection Failed with result code {}".format(rc))
        client.disconnect()
    else:
        print("Connected with result code {}".format(rc))

    client.subscribe("{base_topic}{device_id}/$state".format(**userdata))  # v3 / v4 devices
    client.subscribe("{base_topic}{device_id}/$online".format(**userdata))  # v2 devices


    print("Waiting for device to come online...")


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # decode string for python2/3 compatiblity
    msg.payload = msg.payload.decode()

    if msg.topic.endswith('$implementation/ota/status'):
        status = int(msg.payload.split()[0])

        if userdata.get("published"):
            if status == 202:
                print("Checksum accepted")
            elif status == 206: # in progress
                # state in progress, print progress bar
                progress, total = [int(x) for x in msg.payload.split()[1].split('/')]
                bar_width = 30
                bar = int(bar_width*(progress/total))
                print("\r[", '+'*bar, ' '*(bar_width-bar), "] ", msg.payload.split()[1], end='', sep='')
                if (progress == total):
                    print()
                sys.stdout.flush()
            elif status == 304: # not modified
                print("Device firmware already up to date with md5 checksum: {}".format(userdata.get('md5')))
                client.disconnect()
            elif status == 403: # forbidden
                print("Device ota disabled, aborting...")
                client.disconnect()
            elif (status > 300) and (status < 500):
                print("Other error '" + msg.payload + "', aborting...")
                client.disconnect()
            else:
                print("Other error '" + msg.payload + "'")

    elif msg.topic.endswith('$fw/checksum'):
        checksum = msg.payload

        if userdata.get("published"):
            if checksum == userdata.get('md5'):
                print("Device back online. Update Successful!")
            else:
                print("Expecting checksum {}, got {}, update failed!".format(userdata.get('md5'), checksum))
            client.disconnect()
        else:
            if checksum != userdata.get('md5'): # save old md5 for comparison with new firmware
                userdata.update({'old_md5': checksum})
            else:
                print("Device firmware already up to date with md5 checksum: {}".format(checksum))
                client.disconnect()

    elif msg.topic.endswith('ota/enabled'):
        if msg.payload == 'true':
            userdata.update({'ota_enabled': True})
        else:
            print("Device ota disabled, aborting...")
            client.disconnect()

    elif msg.topic.endswith('$state') or msg.topic.endswith('$online'):
        if (msg.topic.endswith('$state') and msg.payload != 'ready') or (msg.topic.endswith('$online') and msg.payload == 'false'): 
            return

        # calcluate firmware md5
        firmware_md5 = md5(userdata['firmware']).hexdigest()
        userdata.update({'md5': firmware_md5})

        # Subscribing in on_connect() means that if we lose the connection and
        # reconnect then subscriptions will be renewed.
        client.subscribe("{base_topic}{device_id}/$implementation/ota/status".format(**userdata))
        client.subscribe("{base_topic}{device_id}/$implementation/ota/enabled".format(**userdata))
        client.subscribe("{base_topic}{device_id}/$fw/#".format(**userdata))

        # Wait for device info to come in and invoke the on_message callback where update will continue
        print("Waiting for device info...")

    if ( not userdata.get("published") ) and ( userdata.get('ota_enabled') ) and \
       ( 'old_md5' in userdata.keys() ) and ( userdata.get('md5') != userdata.get('old_md5') ):
        # push the firmware binary
        userdata.update({"published": True})
        topic = "{base_topic}{device_id}/$implementation/ota/firmware/{md5}".format(**userdata)
        print("Publishing new firmware with checksum {}".format(userdata.get('md5')))
        client.publish(topic, userdata['firmware'])


def main(broker_host, broker_port, broker_username, broker_password, broker_ca_cert, base_topic, device_id, firmware):
    # initialise mqtt client and register callbacks
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # set username and password if given
    if broker_username and broker_password:
        client.username_pw_set(broker_username, broker_password)

    if broker_ca_cert is not None:
        client.tls_set(
            ca_certs=broker_ca_cert
        )

    # save data to be used in the callbacks
    client.user_data_set({
            "base_topic": base_topic,
            "device_id": device_id,
            "firmware": firmware
        })

    # start connection
    print("Connecting to mqtt broker {} on port {}".format(broker_host, broker_port))
    client.connect(broker_host, broker_port, 60)

    # Blocking call that processes network traffic, dispatches callbacks and handles reconnecting.
    client.loop_forever()


if __name__ == '__main__':
    import argparse

    print (sys.argv[1:])

    parser = argparse.ArgumentParser(
        description='ota firmware update scirpt for ESP8226 implemenation of the Homie mqtt IoT convention.')

    # ensure base topic always ends with a '/'
    def base_topic_arg(s):
        s = str(s)
        if not s.endswith('/'):
            s = s + '/'
        return s

    # specify arguments
    parser.add_argument('-l', '--broker-host',     type=str,            required=False,
                        help='host name or ip address of the mqtt broker', default="127.0.0.1")
    parser.add_argument('-p', '--broker-port',     type=int,            required=False,
                        help='port of the mqtt broker', default=1883)
    parser.add_argument('-u', '--broker-username', type=str,            required=False,
                        help='username used to authenticate with the mqtt broker')
    parser.add_argument('-d', '--broker-password', type=str,            required=False,
                        help='password used to authenticate with the mqtt broker')
    parser.add_argument('-t', '--base-topic',      type=base_topic_arg, required=False,
                        help='base topic of the homie devices on the broker', default="homie/")
    parser.add_argument('-i', '--device-id',       type=str,            required=True,
                        help='homie device id')
    parser.add_argument('firmware', type=argparse.FileType('rb'),
                        help='path to the firmware to be sent to the device')

    parser.add_argument("--broker-tls-cacert", default=None, required=False,
                        help="CA certificate bundle used to validate TLS connections. If set, TLS will be enabled on the broker conncetion"
    )

    # workaround for http://bugs.python.org/issue9694
    parser._optionals.title = "arguments"

    # get and validate arguments
    args = parser.parse_args()

    # read the contents of firmware into buffer
    fw_buffer = args.firmware.read()
    args.firmware.close()
    firmware = bytearray()
    firmware.extend(fw_buffer)

    # Invoke the business logic
    main(args.broker_host, args.broker_port, args.broker_username,
         args.broker_password, args.broker_tls_cacert, args.base_topic, args.device_id, firmware)

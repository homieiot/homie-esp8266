#!/usr/bin/env python

import re
import sys

if len(sys.argv) != 2:
  print("Please specify a file")
  sys.exit(1)

regex_homie = re.compile(b"\x25\x48\x4f\x4d\x49\x45\x5f\x45\x53\x50\x38\x32\x36\x36\x5f\x46\x57\x25")
regex_name = re.compile(b"\xbf\x84\xe4\x13\x54(.+)\x93\x44\x6b\xa7\x75")
regex_version = re.compile(b"\x6a\x3f\x3e\x0e\xe1(.+)\xb0\x30\x48\xd4\x1a")
regex_brand = re.compile(b"\xfb\x2a\xf5\x68\xc0(.+)\x6e\x2f\x0f\xeb\x2d")

try:
  firmware_file = open(sys.argv[1], "rb")
except Exception as err:
  print("Error: {0}".format(err.strerror))
  sys.exit(2)

firmware_binary = firmware_file.read()
firmware_file.close()

regex_name_result = regex_name.search(firmware_binary)
regex_version_result = regex_version.search(firmware_binary)

if not regex_homie.search(firmware_binary) or not regex_name_result or not regex_version_result:
  print("Not a valid Homie firmware")
  sys.exit(3)


regex_brand_result = regex_brand.search(firmware_binary)

name = regex_name_result.group(1).decode()
version = regex_version_result.group(1).decode()
brand = regex_brand_result.group(1).decode() if regex_brand_result else "unset (default is Homie)"

print("Name: {0}".format(name))
print("Version: {0}".format(version))
print("Brand: {0}".format(brand))

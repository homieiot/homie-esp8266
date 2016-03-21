#pragma once

#include "../Constants.hpp"

namespace HomieInternals {
  struct ConfigStruct {
    char* name;

    struct WiFi {
      char* ssid;
      char* password;
    } wifi;

    struct Server {
      char* host;
      uint16_t port;
      struct mDNS {
        bool enabled;
        char* service;
      } mdns;
      struct SSL {
        bool enabled;
        char* fingerprint;
      } ssl;
    };

    struct MQTT {
      Server server;
      char* baseTopic;
      bool auth;
      char* username;
      char* password;
    } mqtt;

    struct OTA {
      bool enabled;
      Server server;
      char* path;
    } ota;
  };
}

#pragma once

#include <Arduino.h>
#include <functional>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Bounce2.h>
#include "../../HomieNode.h"
#include "../Constants.hpp"
#include "../Datatypes/SharedInterface.hpp"
#include "../Helpers.hpp"
#include "../Config.hpp"
#include "../Blinker.hpp"
#include "../Logger.hpp"
#include "Boot.hpp"

namespace HomieInternals {
  class BootNormal : public Boot {
    public:
      BootNormal(SharedInterface* shared_interface);
      ~BootNormal();
      void setup();
      void loop();

    private:
      SharedInterface* _shared_interface;
      unsigned long _last_wifi_reconnect_attempt;
      unsigned long _last_mqtt_reconnect_attempt;
      unsigned long _last_signal_sent;
      bool _flagged_for_ota;
      bool _flagged_for_reset;
      char _mqtt_base_topic[16 + 1];
      Bounce _resetDebouncer;
      WiFiClient _wifiClient;
      WiFiClientSecure _wifiClientSecure;

      void _handleReset();
      void _wifiConnect();
      void _mqttConnect();
      void _mqttSetup();
      void _mqttCallback(char* topic, byte* payload, unsigned int length);
  };
}

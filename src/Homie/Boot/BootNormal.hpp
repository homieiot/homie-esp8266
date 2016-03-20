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
      BootNormal(SharedInterface* sharedInterface);
      ~BootNormal();
      void setup();
      void loop();

    private:
      SharedInterface* _sharedInterface;
      unsigned long _lastWifiReconnectAttempt;
      unsigned long _lastMqttReconnectAttempt;
      unsigned long _lastSignalSent;
      bool _setupFunctionCalled;
      bool _wifiConnectNotified;
      bool _wifiDisconnectNotified;
      bool _mqttConnectNotified;
      bool _mqttDisconnectNotified;
      bool _flaggedForOta;
      bool _flaggedForReset;
      char _mqttBaseTopic[16 + 1];
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

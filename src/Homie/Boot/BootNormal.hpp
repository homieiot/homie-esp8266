#pragma once

#include <Arduino.h>
#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Bounce2.h>
#include "../../HomieNode.h"
#include "../Constants.hpp"
#include "../Datatypes/Interface.hpp"
#include "../MqttClient.hpp"
#include "../Helpers.hpp"
#include "../Config.hpp"
#include "../Blinker.hpp"
#include "../Logger.hpp"
#include "Boot.hpp"

namespace HomieInternals {
  class BootNormal : public Boot {
    public:
      BootNormal();
      ~BootNormal();
      void setup();
      void loop();

    private:
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
      Bounce _resetDebouncer;

      void _handleReset();
      void _wifiConnect();
      void _mqttConnect();
      void _mqttSetup();
      void _mqttCallback(char* topic, char* message);
  };
}

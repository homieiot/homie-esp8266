#pragma once

#include <functional>
#include <ESP8266WiFi.h>
#include <Bounce2.h>
#include "../../HomieNode.hpp"
#include "../Constants.hpp"
#include "../Limits.hpp"
#include "../Datatypes/Interface.hpp"
#include "../MqttClient.hpp"
#include "../Helpers.hpp"
#include "../Config.hpp"
#include "../Blinker.hpp"
#include "../Uptime.hpp"
#include "../Timer.hpp"
#include "../Logger.hpp"
#include "Boot.hpp"

extern "C" {
  #include "user_interface.h"
}

namespace HomieInternals {
  class BootNormal : public Boot {
    public:
      BootNormal();
      ~BootNormal();
      void setup();
      void loop();

    private:
      Uptime _uptime;
      Timer _wifiReconnectTimer;
      Timer _mqttReconnectTimer;
      Timer _signalQualityTimer;
      Timer _uptimeTimer;
      bool _setupFunctionCalled;
      bool _wifiConnectNotified;
      bool _wifiDisconnectNotified;
      bool _mqttConnectNotified;
      bool _mqttDisconnectNotified;
      char _otaVersion[MAX_FIRMWARE_VERSION_LENGTH];
      bool _flaggedForOta;
      bool _flaggedForReset;
      Bounce _resetDebouncer;

      void _handleReset();
      void _wifiConnect();
      void _mqttConnect();
      void _mqttSetup();
      void _mqttCallback(char* topic, char* message);
      void _fillMqttTopic(PGM_P topic);
      bool _publishRetainedOrFail(const char* message);
      bool _subscribe1OrFail();
  };
}

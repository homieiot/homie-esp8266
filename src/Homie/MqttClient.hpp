#pragma once

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Datatypes/Interface.hpp"
#include "Logger.hpp"
#include "Constants.hpp"
#include "Limits.hpp"

namespace HomieInternals {
  class MqttClient {
    public:
      MqttClient();
      ~MqttClient();

      void attachInterface(Interface* interface);
      void initMqtt(bool secure);
      char* getTopicBuffer();
      void setCallback(std::function<void(char* topic, char* message)> callback);
      void setServer(const char* host, uint16_t port, const char* fingerprint);
      bool connect(const char* clientId, const char* willMessage, uint8_t willQos, bool willRetain, bool auth = false, const char* username = "", const char* password = "");
      int getState();
      void disconnect();
      bool publish(const char* message, bool retained);
      bool subscribe(uint8_t qos);
      void loop();
      bool connected();

    private:
      Interface* _interface;
      WiFiClient _wifiClient;
      WiFiClientSecure _wifiClientSecure;
      PubSubClient _pubSubClient;
      char _topicBuffer[TOPIC_BUFFER_LENGTH];
      bool _secure;
      const char* _host;
      uint16_t _port;
      const char* _fingerprint;
      uint8_t _subscribeWithoutLoop;

      void _callback(char* topic, uint8_t* payload, uint16_t length);
      std::function<void(char* topic, char* message)> _userCallback;
  };
}

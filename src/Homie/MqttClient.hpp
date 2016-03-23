#pragma once

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Logger.hpp"

namespace HomieInternals {
  class MqttClient {
    public:
      MqttClient();
      ~MqttClient();

      void initMqtt(bool secure);
      void initBuffer(unsigned char maxTopicLength);
      char* getTopicBuffer();
      void setCallback(std::function<void(char* topic, char* message)> callback);
      void setServer(const char* host, unsigned int port, const char* fingerprint);
      bool connect(const char* clientId, const char* willMessage, unsigned char willQos, bool willRetain, bool auth = false, const char* username = nullptr, const char* password = nullptr);
      void disconnect();
      bool publish(const char* message, bool retained);
      bool subscribe(unsigned char qos);
      void loop();
      bool connected();

    private:
      WiFiClient _wifiClient;
      WiFiClientSecure _wifiClientSecure;
      PubSubClient _pubSubClient;
      char* _topicBuffer;
      bool _secure;
      char* _host;
      char* _fingerprint;

      void _callback(char* topic, byte* payload, unsigned int length);
      std::function<void(char* topic, char* message)> _userCallback;
  };
}

/**
 * Homie Node for Pings.
 *
 */

#pragma once

#include <Homie.hpp>

class PingNode : public HomieNode {

public:
  PingNode(const char* id, const char* name, const int measurementInterval = MEASUREMENT_INTERVAL);

protected:
  virtual void setup() override;
  virtual void onReadyToOperate() override;
  virtual bool handleInput(const HomieRange& range, const String& property, const String& value);

  virtual void loop() override;

private:
  static const int MIN_INTERVAL         = 60;  // in seconds
  static const int MEASUREMENT_INTERVAL = 300;

  const char* cCaption = "• Ping Module:";
  const char* cIndent  = "  ◦ ";

  const char* cPing  = "ping";
  const char* cPong  = "pong";
  const char* cHello = "hello";

  unsigned long _measurementInterval;
  unsigned long _lastMeasurement;

  void printCaption();
};

#pragma once

#include "Arduino.h"
#include "StreamingOperator.hpp"
#include "Homie/Datatypes/Interface.hpp"
#include "HomieRange.hpp"

class HomieNode;

namespace HomieInternals {
class SendingPromise {
  friend ::HomieNode;

 public:
  SendingPromise();
  SendingPromise& setQos(uint8_t qos);
  SendingPromise& setRetained(bool retained);
  SendingPromise& overwriteSetter(bool overwrite);
  SendingPromise& setRange(const HomieRange& range);
  SendingPromise& setRange(uint16_t rangeIndex);
  uint16_t send(const String& value);

 private:
  SendingPromise& setNode(const HomieNode& node);
  SendingPromise& setProperty(const String& property);
  const HomieNode* getNode() const;
  const String* getProperty() const;
  uint8_t getQos() const;
  HomieRange getRange() const;
  bool isRetained() const;
  bool doesOverwriteSetter() const;

  const HomieNode* _node;
  const String* _property;
  uint8_t _qos;
  bool _retained;
  bool _overwriteSetter;
  HomieRange _range;
};
}  // namespace HomieInternals

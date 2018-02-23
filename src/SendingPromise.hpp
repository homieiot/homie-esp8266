#pragma once

#include "Arduino.h"
#include "StreamingOperator.hpp"
#include "Homie/Datatypes/Interface.hpp"

class HomieNode;

namespace HomieInternals
{
class SendingPromise
{
public:
  SendingPromise();
  SendingPromise &setQos(uint8_t qos);
  SendingPromise &setRetained(bool retained);
  SendingPromise &overwriteSetter(bool overwrite);
  uint16_t send(const std::string &node_path, const std::string &value);

private:
  uint8_t getQos() const;
  bool isRetained() const;
  bool doesOverwriteSetter() const;

  uint8_t _qos;
  bool _retained;
  bool _overwriteSetter;
};
} // namespace HomieInternals

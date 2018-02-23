#pragma once

#include <functional>
#include <vector>
#include "Arduino.h"
#include "StreamingOperator.hpp"
#include "Homie/Datatypes/Interface.hpp"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/Limits.hpp"
#include "Nodes/NestedNode.hpp"

using Node::NestedNode;
class HomieNode : public NestedNode
{
  friend HomieInternals::HomieClass;

public:
  HomieNode(const char *id, const char *type);
  virtual ~HomieNode();
  const char *getType() const { return _type; }

  //Todo
  // HomieInternals::SendingPromise &setProperty(const String &property) const
  // {

  //   return HomieInternals::Interface::get()
  //       .getSendingPromise()
  //       .setNode(*this)
  //       .setProperty(property)
  //       .setQos(1)
  //       .setRetained(true)
  //       .overwriteSetter(false);
  // }

  static HomieNode *getMaster() { return &MasterHomieNode; }

  static HomieNode MasterHomieNode;

protected:
  const char *_type = 0;
};
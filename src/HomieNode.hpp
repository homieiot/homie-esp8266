#pragma once

#include <functional>
#include <vector>
#include "Arduino.h"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/Limits.hpp"

namespace HomieInternals {
class HomieClass;
class BootNormal;
class BootConfig;

class Property {
  friend BootNormal;

 public:
  explicit Property(const char* id) { _id = id; }
  void settable(PropertyInputHandler inputHandler = [](String value) { return false; }) { _settable = true;  _inputHandler = inputHandler; }

 private:
  const char* getProperty() { return _id; }
  bool isSettable() { return _settable; }
  PropertyInputHandler getInputHandler() { return _inputHandler; }
  const char* _id;
  bool _settable;
  PropertyInputHandler _inputHandler;
};
}  // namespace HomieInternals

class HomieNode {
  friend HomieInternals::HomieClass;
  friend HomieInternals::BootNormal;
  friend HomieInternals::BootConfig;

 public:
  HomieNode(const char* id, const char* type, HomieInternals::NodeInputHandler nodeInputHandler = [](String property, String value) { return false; });

  const char* getId() const { return _id; }
  const char* getType() const { return _type; }

  HomieInternals::Property* advertise(const char* property);

 protected:
  virtual void setup() {}
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual bool handleInput(String const &property, String const &value);

 private:
  const std::vector<HomieInternals::Property*>& getProperties() const;

  static HomieNode* find(const char* id) {
    for (HomieNode* iNode : HomieNode::nodes) {
      if (strcmp(id, iNode->getId()) == 0) return iNode;
    }

    return 0;
  }

  const char* _id;
  const char* _type;
  std::vector<HomieInternals::Property*> _properties;
  HomieInternals::NodeInputHandler _inputHandler;

  static std::vector<HomieNode*> nodes;
};

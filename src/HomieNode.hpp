#pragma once

#include <functional>
#include <vector>
#include "Arduino.h"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/Limits.hpp"
#include "HomieRange.hpp"

namespace HomieInternals {
class HomieClass;
class BootNormal;
class BootConfig;

class Property {
  friend BootNormal;

 public:
  explicit Property(const char* id, bool range = false, uint16_t lower = 0, uint16_t upper = 0) { _id = strdup(id); _range = range; _lower = lower; _upper = upper; }
  void settable(PropertyInputHandler inputHandler = [](HomieRange range, String value) { return false; }) { _settable = true;  _inputHandler = inputHandler; }

 private:
  const char* getProperty() const { return _id; }
  bool isSettable() const { return _settable; }
  bool isRange() const { return _range; }
  uint16_t getLower() const { return _lower; }
  uint16_t getUpper() const { return _upper; }
  PropertyInputHandler getInputHandler() const { return _inputHandler; }
  const char* _id;
  bool _range;
  uint16_t _lower;
  uint16_t _upper;
  bool _settable;
  PropertyInputHandler _inputHandler;
};
}  // namespace HomieInternals

class HomieNode {
  friend HomieInternals::HomieClass;
  friend HomieInternals::BootNormal;
  friend HomieInternals::BootConfig;

 public:
  HomieNode(const char* id, const char* type, HomieInternals::NodeInputHandler nodeInputHandler = [](String property, HomieRange range, String value) { return false; });

  const char* getId() const { return _id; }
  const char* getType() const { return _type; }

  HomieInternals::Property* advertise(const char* property);
  HomieInternals::Property* advertiseRange(const char* property, uint16_t lower, uint16_t upper);

 protected:
  virtual void setup() {}
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual bool handleInput(String const &property, HomieRange range, String const &value);

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

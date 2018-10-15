#pragma once

#include <functional>
#include <vector>
#include "Arduino.h"
#include "StreamingOperator.hpp"
#include "Homie/Datatypes/Interface.hpp"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/Limits.hpp"
#include "HomieRange.hpp"

class HomieNode;

namespace HomieInternals {
class HomieClass;
class Property;
class BootNormal;
class BootConfig;
class SendingPromise;

class PropertyInterface {
  friend ::HomieNode;

 public:
  PropertyInterface();

  void settable(const PropertyInputHandler& inputHandler = [](const HomieRange& range, const String& value) { return false; });

 private:
  PropertyInterface& setProperty(Property* property);

  Property* _property;
};

class Property {
  friend BootNormal;

 public:
  explicit Property(const char* id, bool range = false, uint16_t lower = 0, uint16_t upper = 0) { _id = strdup(id); _range = range; _lower = lower; _upper = upper; _settable = false; }
  void addName(const char* name) { _name = strdup(name); }
  void addUnit(const char* unit) { _unit = strdup(unit); }
  void addDatatype(const char* datatype) { _unit = strdup(datatype); }
  void addFormat(const char* format) { _unit = strdup(format); }
  void settable(const PropertyInputHandler& inputHandler) { _settable = true;  _inputHandler = inputHandler; }

 private:
  const char* getProperty() const { return _id; }
  const char* getName() const { return _name; }
  const char* getUnit() const { return _unit; }
  const char* getDatatype() const { return _datatype; }
  const char* getFormat() const { return _format; }
  bool isSettable() const { return _settable; }
  bool isRange() const { return _range; }
  uint16_t getLower() const { return _lower; }
  uint16_t getUpper() const { return _upper; }
  PropertyInputHandler getInputHandler() const { return _inputHandler; }
  const char* _id;
  const char* _name;
  const char* _unit;
  const char* _datatype:
  const char* _format;
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
  HomieNode(const char* id, const char* name, const char* type, const HomieInternals::NodeInputHandler& nodeInputHandler = [](const String& property, const HomieRange& range, const String& value) { return false; });
  virtual ~HomieNode();

  const char* getId() const { return _id; }
  const char* getType() const { return _type; }
  const char* getName() const {return _name; }

  HomieInternals::PropertyInterface& advertise(const char* property);
  HomieInternals::PropertyInterface& advertiseRange(const char* property, uint16_t lower, uint16_t upper);

  HomieInternals::SendingPromise& setProperty(const String& property) const;

 protected:
  virtual void setup() {}
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual bool handleInput(const String& property, const HomieRange& range, const String& value);

 private:
  const std::vector<HomieInternals::Property*>& getProperties() const;

  static HomieNode* find(const char* id) {
    for (HomieNode* iNode : HomieNode::nodes) {
      if (strcmp(id, iNode->getId()) == 0) return iNode;
    }

    return 0;
  }

  const char* _id;
  const char* _name;
  const char* _type;
  std::vector<HomieInternals::Property*> _properties;
  HomieInternals::NodeInputHandler _inputHandler;

  HomieInternals::PropertyInterface _propertyInterface;

  static std::vector<HomieNode*> nodes;
};

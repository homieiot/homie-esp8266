#pragma once

#include <vector>
#include <functional>
#include "Arduino.h"

#include "./Homie/Datatypes/Callbacks.hpp"

namespace HomieInternals {
class HomieClass;
class Config;
class Validation;
class BootConfig;

class IHomieSetting {
 public:
  static std::vector<IHomieSetting*> settings;

  bool isRequired() const;
  const char* getName() const;
  const char* getDescription() const;

  virtual bool isBool() const { return false; }
  virtual bool isLong() const { return false; }
  virtual bool isDouble() const { return false; }
  virtual bool isConstChar() const { return false; }

  virtual const char* getType() const { return "unknown"; }

 protected:
  explicit IHomieSetting(const char* name, const char* description);
  const char* _name;
  const char* _description;
  bool _required;
  bool _provided;
};
}  // namespace HomieInternals

template <class T>
class HomieSetting : public HomieInternals::IHomieSetting {
  friend HomieInternals::HomieClass;
  friend HomieInternals::Config;
  friend HomieInternals::Validation;
  friend HomieInternals::BootConfig;

 public:
  HomieSetting(const char* name, const char* description);
  T get() const;
  bool wasProvided() const;
  HomieSetting<T>& setDefaultValue(T defaultValue);
  HomieSetting<T>& setValidator(const std::function<bool(T candidate)>& validator);

 private:
  T _value;
  std::function<bool(T candidate)> _validator;

  bool validate(T candidate) const;
  void set(T value);

  bool isBool() const;
  bool isLong() const;
  bool isDouble() const;
  bool isConstChar() const;

  const char* getType() const;
};

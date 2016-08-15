#pragma once

#include <vector>
#include <functional>
#include "Arduino.h"

namespace HomieInternals {
class BootNormal;
class BootConfig;

class IHomieSetting {
 public:
  IHomieSetting() {}

  virtual bool isBool() const { return false; }
  virtual bool isUnsignedLong() const { return false; }
  virtual bool isLong() const { return false; }
  virtual bool isDouble() const { return false; }
  virtual bool isConstChar() const { return false; }

  static std::vector<IHomieSetting*> settings;
};
}  // namespace HomieInternals

template <class T>
class HomieSetting : public HomieInternals::IHomieSetting {
  friend HomieInternals::BootNormal;
  friend HomieInternals::BootConfig;

 public:
  HomieSetting(const char* name, const char* description);
  T get() const;
  bool wasProvided() const;
  HomieSetting<T>& setDefaultValue(T defaultValue);
  HomieSetting<T>& setValidator(std::function<bool(T candidate)> validator);

 private:
  const char* _name;
  const char* _description;
  bool _required;
  bool _provided;
  T _value;
  std::function<bool(T candidate)> _validator;

  bool validate(T candidate) const;
  void set(T value);

  bool isBool() const;
  bool isUnsignedLong() const;
  bool isLong() const;
  bool isDouble() const;
  bool isConstChar() const;
};

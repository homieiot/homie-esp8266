#include "HomieSetting.hpp"

using namespace HomieInternals;

std::vector<std::reference_wrapper<IHomieSetting>> __attribute__((init_priority(101))) IHomieSetting::settings;

IHomieSetting::IHomieSetting(const char * name, const char * description)
  : _name(name)
  , _description(description)
  , _required(true)
  , _provided(false) {
}

bool IHomieSetting::isRequired() const {
  return _required;
}

const char* IHomieSetting::getName() const {
  return _name;
}

const char* IHomieSetting::getDescription() const {
  return _description;
}



template <class T>
HomieSetting<T>::HomieSetting(const char* name, const char* description)
  : IHomieSetting(name, description)
  , _value()
  , _validator([](T candidate) { return true; }) {
  IHomieSetting::settings.push_back(*this);
}

template <class T>
T HomieSetting<T>::get() const {
  return _value;
}

template <class T>
bool HomieSetting<T>::wasProvided() const {
  return _provided;
}

template <class T>
bool HomieSetting<T>::set(T value, bool saveToConfig) {
  if (!_validate(value)) {
    Interface::get().getLogger() << F("✖ Faild to set ") << _name << F(" setting. Reason: Did not pass validator") << endl;
    return false;
  }
  _set(value);
  if (saveToConfig) {
    ValidationResult saveResult = Interface::get().getConfig().saveSetting(_name, _value);
    if (!saveResult.valid) {
      Interface::get().getLogger() << F("✖ Faild to save ") << _name << F(" setting to config file. Reason: ") << saveResult.reason << endl;
      return false;
    }
  }
  Interface::get().getLogger() << F("✔ Saved ") << _name << F(" setting to config file.") << endl;
  return true;
}

template <class T>
HomieSetting<T>& HomieSetting<T>::setDefaultValue(T defaultValue) {
  _value = defaultValue;
  _required = false;
  return *this;
}

template <class T>
HomieSetting<T>& HomieSetting<T>::setValidator(const std::function<bool(T candidate)>& validator) {
  _validator = validator;
  return *this;
}

template <class T>
bool HomieSetting<T>::_validate(T candidate) const {
  return _validator(candidate);
}

template <class T>
void HomieSetting<T>::_set(T value) {
  _value = value;
  _provided = true;
}

template <class T>
bool HomieSetting<T>::_isBool() const { return false; }

template <class T>
bool HomieSetting<T>::_isLong() const { return false; }

template <class T>
bool HomieSetting<T>::_isDouble() const { return false; }

template <class T>
bool HomieSetting<T>::_isConstChar() const { return false; }

template<>
bool HomieSetting<bool>::_isBool() const { return true; }
template<>
const char* HomieSetting<bool>::_getType() const { return "bool"; }

template<>
bool HomieSetting<long>::_isLong() const { return true; }
template<>
const char* HomieSetting<long>::_getType() const { return "long"; }

template<>
bool HomieSetting<double>::_isDouble() const { return true; }
template<>
const char* HomieSetting<double>::_getType() const { return "double"; }

template<>
bool HomieSetting<const char*>::_isConstChar() const { return true; }
template<>
const char* HomieSetting<const char*>::_getType() const { return "string"; }

// Needed because otherwise undefined reference to
template class HomieSetting<bool>;
template class HomieSetting<long>;
template class HomieSetting<double>;
template class HomieSetting<const char*>;

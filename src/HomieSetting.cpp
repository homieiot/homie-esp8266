#include "HomieSetting.hpp"

using namespace HomieInternals;

std::vector<IHomieSetting*> __attribute__((init_priority(101))) IHomieSetting::settings;

HomieInternals::IHomieSetting::IHomieSetting(const char * name, const char * description)
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
  IHomieSetting::settings.push_back(this);
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
bool HomieSetting<T>::validate(T candidate) const {
  return _validator(candidate);
}

template <class T>
void HomieSetting<T>::set(T value) {
  _value = value;
  _provided = true;
}

template <class T>
bool HomieSetting<T>::isBool() const { return false; }

template <class T>
bool HomieSetting<T>::isLong() const { return false; }

template <class T>
bool HomieSetting<T>::isDouble() const { return false; }

template <class T>
bool HomieSetting<T>::isConstChar() const { return false; }

template<>
bool HomieSetting<bool>::isBool() const { return true; }
template<>
const char* HomieSetting<bool>::getType() const { return "bool"; }

template<>
bool HomieSetting<long>::isLong() const { return true; }
template<>
const char* HomieSetting<long>::getType() const { return "long"; }

template<>
bool HomieSetting<double>::isDouble() const { return true; }
template<>
const char* HomieSetting<double>::getType() const { return "double"; }

template<>
bool HomieSetting<const char*>::isConstChar() const { return true; }
template<>
const char* HomieSetting<const char*>::getType() const { return "string"; }

// Needed because otherwise undefined reference to
template class HomieSetting<bool>;
template class HomieSetting<long>;
template class HomieSetting<double>;
template class HomieSetting<const char*>;

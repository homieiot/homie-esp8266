#include "Interface.hpp"

using namespace HomieInternals;

InterfaceData Interface::_interface;  // need to define the static variable

InterfaceData& Interface::get() {
  return _interface;
}

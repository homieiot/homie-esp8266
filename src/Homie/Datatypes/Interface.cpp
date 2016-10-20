#include "Interface.hpp"

using namespace HomieInternals;

InterfaceStruct Interface::_interface;  // need to define the static variable

InterfaceStruct& Interface::get() {
  return _interface;
}

#pragma once

#include "../../HomieEvent.h"
#include <functional>

namespace HomieInternals {
  typedef void (*OperationFunction)();

  typedef bool (*GlobalInputHandler)(String nodeId, String property, String value);
  typedef bool (*NodeInputHandler)(String property, String value);
  typedef std::function<bool(String)> PropertyInputHandler;

  typedef void (*EventHandler)(HomieEvent event);

  typedef bool (*ResetFunction)();
}

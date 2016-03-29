#pragma once

#include "../../HomieEvent.h"

namespace HomieInternals {
  typedef void (*OperationFunction)();

  typedef bool (*GlobalInputHandler)(String nodeId, String property, String value);
  typedef bool (*NodeInputHandler)(String property, String value);
  typedef bool (*PropertyInputHandler)(String value);

  typedef void (*EventHandler)(HomieEvent event);

  typedef bool (*ResetFunction)();
}

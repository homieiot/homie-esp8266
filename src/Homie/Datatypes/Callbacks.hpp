#pragma once

#include "../../HomieEvent.h"

namespace HomieInternals {
  typedef void (*OperationFunction)();

  typedef bool (*GlobalInputHandler)(String nodeId, String property, String message);
  typedef bool (*NodeInputHandler)(String property, String message);
  typedef bool (*PropertyInputHandler)(String message);

  typedef void (*EventHandler)(HomieEvent event);

  typedef bool (*ResetFunction)();
}

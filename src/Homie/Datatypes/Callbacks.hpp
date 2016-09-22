#pragma once

#include "../../HomieEvent.hpp"
#include <functional>

namespace HomieInternals {
  typedef std::function<void()> OperationFunction;

  typedef std::function<bool(String nodeId, String property, String value)> GlobalInputHandler;
  typedef std::function<bool(String property, String value)> NodeInputHandler;
  typedef std::function<bool(String value)> PropertyInputHandler;

  typedef std::function<void(HomieEvent event)> EventHandler;

  typedef std::function<bool()> ResetFunction;
}

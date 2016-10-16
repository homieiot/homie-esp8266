#pragma once

#include <functional>
#include "../../HomieEvent.hpp"
#include "../../HomieRange.hpp"

class HomieNode;

namespace HomieInternals {
  typedef std::function<void()> OperationFunction;

  typedef std::function<bool(const HomieNode& node, String property, HomieRange range, String value)> GlobalInputHandler;
  typedef std::function<bool(String property, HomieRange range, String value)> NodeInputHandler;
  typedef std::function<bool(HomieRange range, String value)> PropertyInputHandler;

  typedef std::function<void(const HomieEvent& event)> EventHandler;

  typedef std::function<bool(String level, String value)> BroadcastHandler;

  typedef std::function<bool()> ResetFunction;
}  // namespace HomieInternals

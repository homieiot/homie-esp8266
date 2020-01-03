#pragma once

#include <functional>
#include "../../HomieEvent.hpp"
#include "../../HomieRange.hpp"

class HomieNode;

namespace HomieInternals {
  typedef std::function<void()> OperationFunction;

  typedef std::function<bool(const HomieNode& node, const HomieRange& range, const String& property, const String& value)> GlobalInputHandler;
  typedef std::function<bool(const HomieRange& range, const String& property, const String& value)> NodeInputHandler;
  typedef std::function<bool(const HomieRange& range, const String& value)> PropertyInputHandler;

  typedef std::function<void(const HomieEvent& event)> EventHandler;

  typedef std::function<bool(const String& level, const String& value)> BroadcastHandler;
}  // namespace HomieInternals

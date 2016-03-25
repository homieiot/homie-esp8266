#pragma once

#include "../Limits.hpp"
#include "./Callbacks.hpp"

namespace HomieInternals {
  struct Subscription {
    char property[MAX_NODE_PROPERTY_LENGTH];
    PropertyInputHandler inputHandler;
  };
}

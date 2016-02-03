#pragma once

namespace HomieInternals {
  struct Subscription {
    char* property;
    bool (*inputHandler)(String message);
  };
}

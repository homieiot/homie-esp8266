#pragma once

namespace HomieInternals {
  typedef struct {
    char* property;
    bool (*inputHandler)(String message);
  } Subscription;
}

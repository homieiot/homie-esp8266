#pragma once

enum class HomieBootMode : uint8_t {
  UNDEFINED = 0,
  STANDALONE = 1,
#if HOMIE_CONFIG
  CONFIGURATION = 2,
#endif
  NORMAL = 3
};

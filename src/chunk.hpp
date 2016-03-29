//
// no-dice
//

#pragma once

#include "types.hpp"

namespace nd {
  using Block = u8;

  struct ChunkData {
    using Shared = SharePtr<ChunkData>;

    Block blocks [16][16][16];
  };
}


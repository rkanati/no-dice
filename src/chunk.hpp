//
// no-dice
//

#pragma once

#include "vec-iter.hpp"
#include "types.hpp"

namespace nd {
  using Block = u8;

  struct ChunkData {
    using Shared = SharePtr<ChunkData>;

    union {
      Block blocks [16][16][16];
      Block flat [16 * 16 * 16];
    };

    Block& operator[] (v3i i) {
      return blocks[i.x][i.y][i.z];
    }

    Block operator[] (v3i i) const {
      return blocks[i.x][i.y][i.z];
    }

    static constexpr auto indices () {
      return vec_range (v3i{0,0,0}, v3i{16,16,16});
    }
  };
}


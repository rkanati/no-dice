//
// no-dice
//

#pragma once

#include "vec-iter.hpp"
#include "types.hpp"

namespace nd {
  using Block = u8;

  enum : int {
    chunk_rel_bits = 6,
    chunk_dim = 1 << chunk_rel_bits,
    chunk_rel_mask = chunk_dim - 1,
    chunk_volume = chunk_dim*chunk_dim*chunk_dim
  };

  struct ChunkData {
    using Shared = SharePtr<ChunkData>;

    union {
      Block blocks [chunk_dim][chunk_dim][chunk_dim];
      Block flat [chunk_volume];
    };

    Block& operator[] (v3i8 i) {
      return blocks[i.x][i.y][i.z];
    }

    Block operator[] (v3i8 i) const {
      return blocks[i.x][i.y][i.z];
    }

    Block& operator[] (int i) {
      return flat[i];
    }

    Block operator[] (int i) const {
      return flat[i];
    }

    static constexpr auto indices () {
      return vec_range (v3i8{0,0,0}, v3i8{chunk_dim,chunk_dim,chunk_dim});
    }
  };
}


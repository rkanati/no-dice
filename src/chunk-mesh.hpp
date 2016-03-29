//
// no-dice
//

#pragma once

#include "types.hpp"
#include "chunk.hpp"

namespace nd {
  class ChunkMesh {
  public:
    using Shared = SharePtr<ChunkMesh>;
    static Shared generate (ChunkData const* chunk, Array<3, ChunkData const*> const& adjs);

    virtual void draw () const = 0;
  };
}


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
    static Shared create ();

    virtual void regen (ChunkData const* chunk, Array<3, ChunkData const*> const& adjs) = 0;
    virtual void draw () const = 0;
  };
}


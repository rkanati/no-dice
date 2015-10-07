//
// no-dice
//

#pragma once

#include "chunk.hpp"

namespace nd {
  class ChunkMesh {
    struct Impl;
    Impl* impl;
    ChunkMesh (Impl*);

  public:
    static ChunkMesh generate (const Chunk&, const Chunk&, const Chunk&, const Chunk&);
    ~ChunkMesh ();

    void draw () const;
  };
}


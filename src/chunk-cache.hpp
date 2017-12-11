//
// no-dice
//

#pragma once

#include "chunk-source.hpp"
#include "vector.hpp"
#include "chunk.hpp"

#include <memory>

namespace nd {
  class ChunkCache {
    struct Impl;
    Impl* impl;

  public:
    ChunkCache ();
    ~ChunkCache ();

    ChunkData::Shared load  (v3i chunk_pos) const;
    void              store (v3i chunk_pos, ChunkData::Shared);
  };
}


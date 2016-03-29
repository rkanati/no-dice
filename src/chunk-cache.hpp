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
    ChunkCache (ChunkSource& src);
    ~ChunkCache ();

    auto load (vec3i chunk_pos) -> std::shared_ptr <ChunkData>;
    void store (vec3i chunk_pos, const ChunkData& chunk);
  };
}


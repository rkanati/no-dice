//
// no-dice
//

#pragma once

#include "vector.hpp"
#include "chunk.hpp"

#include <memory>

namespace nd {
  class ChunkSource {
  public:
    virtual auto get   (vec3i chunk_pos) -> Shared<ChunkData> = 0;
    virtual void store (vec3i chunk_pos, Shared<ChunkData>) = 0;
  };
}


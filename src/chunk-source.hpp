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
    virtual auto load  (vec3i chunk_pos) -> std::shared_ptr <ChunkData>;
    virtual void store (vec3i chunk_pos, const ChunkData&);
  };
}


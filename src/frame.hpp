
#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "chunk-mesh.hpp"

#include <vector>

namespace nd {
  class Frame {
    struct ChunkItem {
      ChunkMesh const* mesh;
      vec3i offset;
    };

    std::vector<ChunkItem> chunk_items;

  public:
    Frame () = default;

    void add_cmesh (ChunkMesh const* cmesh, vec3i offset) {
      chunk_items.push_back (ChunkItem { cmesh, offset });
    }

    void draw (v2i dims, float time, float alpha);
  };
}


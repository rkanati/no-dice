
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

    explicit Frame (std::vector<ChunkItem> chunk_items) :
      chunk_items (std::move (chunk_items))
    {
      chunk_items.clear ();
    }

  public:
    Frame () = default;

    static inline Frame recycle (Frame& old) {
      return Frame { std::move (old.chunk_items) };
    }

    void add_cmesh (ChunkMesh const* cmesh, vec3i offset) {
      chunk_items.push_back (ChunkItem { cmesh, offset });
    }

    Frame draw (v2i dims, float time, float alpha);
  };
}


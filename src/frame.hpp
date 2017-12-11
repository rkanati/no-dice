
#pragma once

#include "types.hpp"
#include "placement.hpp"
#include "chunk-mesh.hpp"
#include "shader.hpp"
#include "frustum.hpp"

#include <vector>

namespace nd {
  class Frame {
    ChunkRenderer::Shared chunk_renderer;
    m4f w2e, e2c;
    std::vector<ChunkRenderer::Item> chunk_items;

  public:
    Frame ();

    void set_frustum (Frustum f) { w2e = f.w2e; e2c = f.e2c; }

    void add_chunk (ChunkMesh const& mesh, v3i offset, int z) {
      ChunkRenderer::Item item { mesh.get (), offset, z };
      chunk_items.push_back (item);
    }

    void draw (v2i dims, float time, float alpha);
  };
}


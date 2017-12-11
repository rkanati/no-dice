//
// no-dice
//

#pragma once

#include "types.hpp"
#include "chunk.hpp"

#include <Rk/matrix.hpp>

namespace nd {
  struct ChunkMeshImpl;
  using ChunkMesh = std::shared_ptr<ChunkMeshImpl>;

  using AdjChunkData = std::array<ChunkData const*, 4>;

  class ChunkMesher {
  public:
    using Shared = std::shared_ptr<ChunkMesher>;
    virtual ChunkMesh build (AdjChunkData const&) = 0;
  };

  auto make_chunk_mesher () -> ChunkMesher::Shared;

  class ChunkRenderer {
  public:
    struct Item {
      ChunkMeshImpl const* mesh;
      v3i offset;
      int z;
    };

    using Shared = std::shared_ptr<ChunkRenderer>;
    virtual void draw (
      m4f const& w2c,
      std::vector<Item> const&
    ) = 0;
  };

  auto make_chunk_renderer () -> ChunkRenderer::Shared;
}


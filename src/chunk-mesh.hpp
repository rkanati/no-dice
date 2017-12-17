//
// no-dice
//

#pragma once

#include "types.hpp"
#include "chunk.hpp"

#include <Rk/matrix.hpp>

namespace nd {
  struct ChunkMeshImpl;
  using ChunkMesh = Shared<ChunkMeshImpl>;

  using AdjChunkData = std::array<ChunkData const*, 4>;

  class ChunkMesher {
  public:
    virtual ChunkMesh build (AdjChunkData const&) = 0;
  };

  auto make_chunk_mesher () -> Shared<ChunkMesher>;

  class ChunkRenderer {
  public:
    struct Item {
      ChunkMeshImpl const* mesh;
      v3i offset;
      int z;
    };

    virtual void draw (
      m4f const& w2c,
      std::vector<Item> const&
    ) = 0;
  };

  auto make_chunk_renderer () -> Shared<ChunkRenderer>;
}


//
// no-dice
//

#pragma once

#include "chunk.hpp"
#include "chunk-mesh.hpp"
#include "types.hpp"
#include "maybe.hpp"

namespace nd {
  class StageChunk {
  public:
    ChunkData::Shared data;
    ChunkMesh::Shared mesh;
    vec3i             position;
    bool              mesh_ok;

    StageChunk () :
      mesh (ChunkMesh::create ()),
      position (nil),
      mesh_ok (false)
    { }

    StageChunk (ChunkData::Shared data, vec3i position) :
      data (std::move (data)),
      mesh (ChunkMesh::create ()),
      position (position),
      mesh_ok (false)
    { }

    StageChunk (const StageChunk&) = delete;
    StageChunk (StageChunk&&) = default;
    StageChunk& operator = (const StageChunk&) = delete;
    StageChunk& operator = (StageChunk&&) = default;

    void regen_mesh (Array<3, ChunkData const*> const& adjs) {
      mesh->regen (data.get (), adjs);
      mesh_ok = true;
    }

    explicit operator bool () const {
      return (bool) data;
    }
  };
}


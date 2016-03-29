
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

    StageChunk () :
      position (nil)
    { }

    StageChunk (ChunkData::Shared data, vec3i position) :
      data (std::move (data)),
      position (position)
    { }

    StageChunk (const StageChunk&) = delete;
    StageChunk (StageChunk&&) = default;
    StageChunk& operator = (const StageChunk&) = delete;
    StageChunk& operator = (StageChunk&&) = default;

    explicit operator bool () const {
      return (bool) data;
    }
  };
}


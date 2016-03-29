
#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "chunk-mesh.hpp"

#include <vector>

namespace nd {
  class Frame {
    std::vector <ChunkMesh const*> cmeshes;

  public:
    explicit Frame (std::vector <ChunkMesh const*> cms = { }) :
      cmeshes (std::move (cms))
    {
      cmeshes.clear ();
    }

    static inline Frame recycle (Frame& old) {
      return Frame { std::move (old.cmeshes) };
    }

    void add_cmesh (ChunkMesh const* cmesh) {
      cmeshes.push_back (cmesh);
    }

    Frame draw (v2i dims, float time, float alpha);
  };
}


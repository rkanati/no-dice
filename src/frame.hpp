
#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "chunk-mesh.hpp"

#include <vector>

namespace nd {
  struct Camera {
    v3f pos, dir;

    Camera () = default;

    Camera (nil_t) :
      pos (nil), dir (nil)
    { }

    Camera (v3f pos, v3f dir) :
      pos (pos), dir (dir)
    { }
  };

  static Camera lerp (Camera const& from, Camera const& to, float alpha) {
    return {
      lerp (from.pos, to.pos, alpha),
      lerp (from.dir, to.dir, alpha)
    };
  }

  class Frame {
    Camera old_camera, cur_camera;

    struct ChunkItem {
      ChunkMesh const* mesh;
      vec3i offset;
    };

    std::vector<ChunkItem> chunk_items;

  public:
    Frame () = default;

    void set_camera (Camera old, Camera cur) {
      old_camera = old;
      cur_camera = cur;
    }

    void add_cmesh (ChunkMesh const* cmesh, vec3i offset) {
      chunk_items.push_back (ChunkItem { cmesh, offset });
    }

    void draw (v2i dims, float time, float alpha);
  };
}


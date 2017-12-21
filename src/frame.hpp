
#pragma once

#include "chunk-mesh.hpp"
#include "placement.hpp"
#include "frustum.hpp"
#include "shader.hpp"
#include "types.hpp"

#include <vector>

namespace nd {
  struct UIVertex {
    v2i pos, uv;
    UIVertex () = default;
    UIVertex (v2i pos, v2i uv) : pos (pos), uv (uv) { }
  };

  struct UIRect {
    UIVertex verts[6];
    UIRect () = default;
    UIRect (UIVertex v0, UIVertex v1, UIVertex v2, UIVertex v3) :
      verts { v0, v1, v2, v3, v0, v2 }
    { }
  };

  class Frame {
    Shared<ChunkRenderer> chunk_renderer;

    ShaderProgram ui_prog;
    uint ui_vao, ui_buf;

    m4f w2e, e2c;

    std::vector<ChunkRenderer::Item> chunk_items;

    struct UIItem {
      v2i  pos;
      float k;
      uint texture, begin, count;
      bool operator < (UIItem const& other) const {
        if (texture < other.texture)
          return true;
        else if (texture > other.texture)
          return false;
        else
          return begin < other.begin;
      }
    };

    std::vector<UIRect> ui_rects;
    std::vector<UIItem> ui_items;

  public:
    Frame ();

    void set_frustum (Frustum f) { w2e = f.w2e; e2c = f.e2c; }

    void add_chunk (ChunkMesh const& mesh, v3i const offset, int const z) {
      ChunkRenderer::Item item { mesh.get (), offset, z };
      chunk_items.push_back (item);
    }

    void add_rects (
      uint const texture, v2i const pos,
      UIRect const* const begin, UIRect const* const end,
      float k = 1.f)
    {
      ui_items.push_back (
        UIItem { pos, k, texture, (uint) ui_rects.size (), uint (end-begin) }
      );

      ui_rects.insert (ui_rects.end (), begin, end);
    }

    void draw (v2i dims, float time, float alpha);
  };
}


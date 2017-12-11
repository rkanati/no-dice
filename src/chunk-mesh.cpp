//
// no-dice
//

#include "chunk-mesh.hpp"

#include "vector.hpp"
#include "shader.hpp"

#include <Rk/transform.hpp>

#include <iostream>
#include <algorithm>
#include <vector>

#include <epoxy/gl.h>

namespace nd {
  size_t const constexpr
    max_faces = 3 * chunk_volume,
    max_verts = 4 * max_faces;

  struct Strip {
    v3i8 xyz;
    u8   tex;
    int  length;
  };

  struct alignas (8) Vertex {
    v3i8 xyz;
    u8   dir,
         tex;
    Vertex () = default;
    Vertex (v3i8 xyz, u8 dir, u8 tex) :
      xyz (xyz), dir (dir), tex (tex) { }
  };

  struct Quad {
    Vertex verts[4];
  };

  struct ChunkMeshImpl {
    uint vao, n_faces, query;

    ChunkMeshImpl (uint vao, uint n_faces, uint query) :
      vao (vao),
      n_faces (n_faces),
      query (query)
    { }

    ~ChunkMeshImpl () {
      glDeleteVertexArrays (1, &vao);
      glDeleteQueries (1, &query);
    }
  };

  // chunk mesher
  class ChunkMesherImpl final : public ChunkMesher {
    uint index_buffer = 0;

    std::vector<Vertex> tiles[6][chunk_dim];
    std::vector<Strip>  strips;
    std::vector<Quad>   quads;

    void bind_indices ();

    ChunkMesh build (AdjChunkData const&) override;

  public:
    ChunkMesherImpl ();
    ~ChunkMesherImpl ();
  };

  auto make_chunk_mesher () -> ChunkMesher::Shared {
    return std::make_shared<ChunkMesherImpl> ();
  }

  void ChunkMesherImpl::bind_indices () {
    if (index_buffer == 0) {
      std::vector<u32> indices;
      indices.resize (6*max_faces);
      for (u32 i = 0, j = 0; i != max_verts; i += 4, j += 6) {
        indices[j + 0] = i + 0;
        indices[j + 1] = i + 1;
        indices[j + 2] = i + 2;
        indices[j + 3] = i + 2;
        indices[j + 4] = i + 3;
        indices[j + 5] = i + 0;
      }

      //glCreateBuffers (1, &index_buffer);
      glGenBuffers (1, &index_buffer);
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, index_buffer);
      glBufferData (
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size () * 4, indices.data (),
        GL_STATIC_DRAW
      );
    }
    else {
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    }
  }

  ChunkMesherImpl::ChunkMesherImpl () {
    for (auto& dir : tiles) {
      for (auto& plane : dir)
        plane.reserve (chunk_dim*chunk_dim);
    }
  }

  ChunkMesherImpl::~ChunkMesherImpl () {
    glDeleteBuffers (1, &index_buffer);
  }

  ChunkMesh ChunkMesherImpl::build (AdjChunkData const& adjs) {
    for (auto& dir : tiles) {
      for (auto& plane : dir)
        plane.clear ();
    }

    // build tiles
    auto const& chunk = *adjs[0];
    for (auto i : chunk.indices ()) {
      Block const
        current = chunk[i],
        zadj = (i.z==chunk_dim-1)? (*adjs[3])[v3i8(i.x,i.y,0    )]
                                 : chunk     [v3i8(i.x,i.y,i.z+1)],
        yadj = (i.y==chunk_dim-1)? (*adjs[2])[v3i8(i.x,0,    i.z)]
                                 : chunk     [v3i8(i.x,i.y+1,i.z)],
        xadj = (i.x==chunk_dim-1)? (*adjs[1])[v3i8(0,    i.y,i.z)]
                                 : chunk     [v3i8(i.x+1,i.y,i.z)];

      if (current != 0) {
        if (zadj == 0) tiles[0][i.z].emplace_back (i, 0, current);
        if (yadj == 0) tiles[1][i.y].emplace_back (i, 1, current);
        if (xadj == 0) tiles[2][i.x].emplace_back (i, 2, current);
      }
      else {
        if (zadj != 0) tiles[3][i.z].emplace_back (i, 3, zadj);
        if (yadj != 0) tiles[4][i.y].emplace_back (i, 4, yadj);
        if (xadj != 0) tiles[5][i.x].emplace_back (i, 5, xadj);
      }
    }

    // compile tiles into strips
    v3i constexpr const bases[6][3] = {
      { v3i{0,1,0}, v3i{1,0,0}, v3i{0,0,1} },
      { v3i{0,0,1}, v3i{1,0,0}, v3i{0,1,0} },
      { v3i{0,0,1}, v3i{0,1,0}, v3i{1,0,0} },
      { v3i{0,1,0}, v3i{1,0,0}, v3i{0,0,1} },
      { v3i{0,0,1}, v3i{1,0,0}, v3i{0,1,0} },
      { v3i{0,0,1}, v3i{0,1,0}, v3i{1,0,0} }
    };

    v3i constexpr const proto_quads[6][4] {
      { v3i{1,0,1}, v3i{1,1,1}, v3i{0,1,1}, v3i{0,0,1} },
      { v3i{0,1,0}, v3i{0,1,1}, v3i{1,1,1}, v3i{1,1,0} },
      { v3i{1,1,0}, v3i{1,1,1}, v3i{1,0,1}, v3i{1,0,0} },
      { v3i{1,1,1}, v3i{1,0,1}, v3i{0,0,1}, v3i{0,1,1} },
      { v3i{1,1,0}, v3i{1,1,1}, v3i{0,1,1}, v3i{0,1,0} },
      { v3i{1,0,0}, v3i{1,0,1}, v3i{1,1,1}, v3i{1,1,0} }
    };

    quads.clear ();

    // for each orientation
    for (int dir = 0; dir != 6; dir++) {
      auto basis = bases[dir];
      auto const& proto = proto_quads[dir];

      // for each plane
      for (int pi = 0; pi != chunk_dim; pi++) {
        auto const& plane = tiles[dir][pi];

        strips.clear ();

        // for each tile in the plane
        for (int ti = 0; ti != (int) plane.size ();) {
          auto u = plane[ti++];
          int length = 1;
          auto prev_xyz = u.xyz;

          // extend strip over similar tiles
          while (ti != (int) plane.size ()) {
            auto v = plane[ti];
            if (u.tex != v.tex || v.xyz - prev_xyz != basis[0])
              break;
            prev_xyz = v.xyz;
            length++;
            ti++;
          }

          // build strip
          strips.push_back (Strip { u.xyz, u.tex, length });
        }

        // sort strips
        auto comp = [basis] (Strip const& a, Strip const& b) {
          int ad = dot (a.xyz, basis[0]), bd = dot (b.xyz, basis[0]);
          if (ad < bd)
            return true;
          else if (bd < ad)
            return false;
          else
            return dot (a.xyz, basis[1]) < dot (b.xyz, basis[1]);
        };
        std::sort (strips.begin (), strips.end (), comp);

        // coalesce strips
        for (int si = 0; si != (int) strips.size ();) {
          auto s = strips[si++];
          int width = 1;
          auto prev_xyz = s.xyz;

          while (si != (int) strips.size ()) {
            auto t = strips[si];
            bool similar = (s.tex == t.tex) && (s.length == t.length),
                 adjacent = t.xyz - prev_xyz == basis[1];
            if (!similar || !adjacent)
              break;
            prev_xyz = t.xyz;
            width++;
            si++;
          }

          // build quad
          v3i scale
            = s.length * basis[0]
            + width    * basis[1]
            +            basis[2];
          Quad q {
            Vertex { s.xyz + proto[0]*scale, (u8) dir, s.tex },
            Vertex { s.xyz + proto[1]*scale, (u8) dir, s.tex },
            Vertex { s.xyz + proto[2]*scale, (u8) dir, s.tex },
            Vertex { s.xyz + proto[3]*scale, (u8) dir, s.tex }
          };
          quads.push_back (q);
        }
      }
    }

    if (quads.empty ())
      return nullptr;

    // upload quads
    uint vao = 0;
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);

    { uint buffer = 0;
      glGenBuffers (1, &buffer);
      glBindBuffer (GL_ARRAY_BUFFER, buffer);

      auto pitch = sizeof (Vertex);
      #define VOFF(field) ((void const*) offsetof (Vertex, field))
      glVertexAttribPointer (1, 3, GL_BYTE, GL_FALSE, pitch, VOFF (xyz));
      glEnableVertexAttribArray (1);

      glVertexAttribIPointer (2, 1, GL_UNSIGNED_BYTE, pitch, VOFF (dir));
      glEnableVertexAttribArray (2);

      glVertexAttribIPointer (3, 1, GL_UNSIGNED_BYTE, pitch, VOFF (tex));
      glEnableVertexAttribArray (3);

      glBufferData (
        GL_ARRAY_BUFFER,
        quads.size () * sizeof (Quad), quads.data (),
        GL_STATIC_DRAW
      );
    }

    bind_indices ();

    glBindVertexArray (0);

    // occlusion query
    uint query;
    glGenQueries (1, &query);

  /*if (!quads.empty ())
      std::cerr << "nonempty mesh...\n";*/
    return std::make_shared<ChunkMeshImpl> (vao, quads.size (), query);
  }

  // chunk renderer
  class ChunkRendererImpl final : public ChunkRenderer {
    uint cube_vao = 0;
    ShaderProgram cube_prog, mesh_prog;

    void draw (
      m4f const& w2c,
      std::vector<Item> const&
    ) override;

  public:
    ChunkRendererImpl ();
    ~ChunkRendererImpl ();
  };

  auto make_chunk_renderer () -> ChunkRenderer::Shared {
    return std::make_shared<ChunkRendererImpl> ();
  }

  ChunkRendererImpl::ChunkRendererImpl () {
    cube_prog = link (
      load_shader ("assets/shaders/occluder.vert", ShaderType::vertex)
    );

    mesh_prog = link (
      load_shader ("assets/shaders/terrain.vert", ShaderType::vertex),
      load_shader ("assets/shaders/terrain.frag", ShaderType::fragment)
    );

    { int const c = chunk_dim;
      v3i8 const verts[8] {
        v3i8{0,0,0}, v3i8{0,c,0}, v3i8{c,c,0}, v3i8{c,0,0},
        v3i8{0,0,c}, v3i8{c,0,c}, v3i8{c,c,c}, v3i8{0,c,c}
      };
      u8 const idxs[22] {
           0,1,3,2, 2, // bottom
        4, 4,5,7,6, 6, // top
        0, 0,4, 1,7, 2,6, 3,5, 0,4
      };

      glGenVertexArrays (1, &cube_vao);
      glBindVertexArray (cube_vao);

      uint buf;
      glGenBuffers (1, &buf);
      glBindBuffer (GL_ARRAY_BUFFER, buf);
      glBufferData (GL_ARRAY_BUFFER, sizeof (verts), verts, GL_STATIC_DRAW);
      glVertexAttribPointer (1, 3, GL_BYTE, GL_FALSE, 3, 0);
      glEnableVertexAttribArray (1);

      glGenBuffers (1, &buf);
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, buf);
      glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (idxs), idxs, GL_STATIC_DRAW);

      glBindVertexArray (0);
    }
  }

  ChunkRendererImpl::~ChunkRendererImpl () {
    glDeleteVertexArrays (1, &cube_vao);
  }

  void ChunkRendererImpl::draw (m4f const& w2c, std::vector<Item> const& items) {
    mesh_prog.use ();

  /*glBlendFunc (GL_ONE, GL_DST_ALPHA);
    glEnable (GL_BLEND);*/

    for (auto const& item : items) {
      auto xform = w2c * Rk::translation (item.offset);
      glUniformMatrix4fv (1, 1, GL_TRUE, xform.raw ());

      // do occlusion query
    /*glColorMask (false, false, false, false);
      glDepthMask (false);
      glDisable (GL_CULL_FACE);
      glBindVertexArray (cube_vao);
      glBeginQuery (GL_SAMPLES_PASSED, item.mesh->query);
      glDrawElements (GL_TRIANGLE_STRIP, 22, GL_UNSIGNED_BYTE, nullptr);
      glEndQuery (GL_SAMPLES_PASSED);*/

      // draw
    /*glColorMask (true, true, true, true);
      glDepthMask (true);
      glEnable (GL_CULL_FACE);*/
      glBindVertexArray (item.mesh->vao);
    //glBeginConditionalRender (item.mesh->query, GL_QUERY_WAIT);
      auto const n = 6 * item.mesh->n_faces;
      glDrawElements (GL_TRIANGLES, n, GL_UNSIGNED_INT, nullptr);
    //glEndConditionalRender ();
    }
  }
}


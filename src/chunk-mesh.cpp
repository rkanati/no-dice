//
// no-dice
//

#include "chunk-mesh.hpp"

#include "vector.hpp"

#include <vector>

#include <GL/gl.h>

namespace nd {
  struct Vertex {
    v2f uv;
    v3f rgb, xyz;
  };

  class ChunkMeshImpl final : public ChunkMesh {
    std::vector<Vertex> arrays;

  public:
    ChunkMeshImpl () {
      arrays.reserve (4 * 3 * 16 * 16 * 16);
    }

    void regen (ChunkData const* chunk, Array<3, ChunkData const*> const& adjs) override;
    void draw () const override;
  };

  void ChunkMeshImpl::draw () const {
    auto* const base = (u8*) arrays.data ();
    glTexCoordPointer (2, GL_FLOAT, sizeof (Vertex), base + offsetof (Vertex, uv));
    glColorPointer    (3, GL_FLOAT, sizeof (Vertex), base + offsetof (Vertex, rgb));
    glVertexPointer   (3, GL_FLOAT, sizeof (Vertex), base + offsetof (Vertex, xyz));

    glDrawArrays (GL_QUADS, 0, arrays.size ());
  }

  void ChunkMeshImpl::regen (ChunkData const* chunk, Array<3, ChunkData const*> const& adjs) {
    arrays.clear ();

    for (int z = 0; z != 16; z++) {
      auto zadj = (z != 15)? chunk->blocks : adjs[2]->blocks;

      for (int y = 0; y != 16; y++) {
        auto yadj = (y != 15)? chunk->blocks : adjs[1]->blocks;

        for (int x = 0; x != 16; x++) {
          auto xadj = (x != 15)? chunk->blocks : adjs[0]->blocks;

          Block current = chunk->blocks[x][y][z];

          static v3f const xcol { 0.8f, 0.8f, 0.8f },
                           ycol { 0.6f, 0.6f, 0.6f },
                           zcol { 1.0f, 1.0f, 1.0f };

          if (current != 0) { // face out
            if (xadj [(x+1)%16][y][z] == 0) { // +x
              arrays.push_back (Vertex { v2f {0,0}, xcol, v3i {x+1, y+0, z+0} });
              arrays.push_back (Vertex { v2f {1,0}, xcol, v3i {x+1, y+1, z+0} });
              arrays.push_back (Vertex { v2f {1,1}, xcol, v3i {x+1, y+1, z+1} });
              arrays.push_back (Vertex { v2f {0,1}, xcol, v3i {x+1, y+0, z+1} });
            }
            if (yadj [x][(y+1)%16][z] == 0) { // +y
              arrays.push_back (Vertex { v2f {1,0}, ycol, v3i {x+0, y+1, z+0} });
              arrays.push_back (Vertex { v2f {1,1}, ycol, v3i {x+0, y+1, z+1} });
              arrays.push_back (Vertex { v2f {0,1}, ycol, v3i {x+1, y+1, z+1} });
              arrays.push_back (Vertex { v2f {0,0}, ycol, v3i {x+1, y+1, z+0} });
            }
            if (zadj [x][y][(z+1)%16] == 0) { // +z
              arrays.push_back (Vertex { v2f {1,0}, zcol, v3i {x+0, y+0, z+1} });
              arrays.push_back (Vertex { v2f {1,1}, zcol, v3i {x+1, y+0, z+1} });
              arrays.push_back (Vertex { v2f {0,1}, zcol, v3i {x+1, y+1, z+1} });
              arrays.push_back (Vertex { v2f {0,0}, zcol, v3i {x+0, y+1, z+1} });
            }
          }
          else if (current == 0) { // face in
            if (xadj [(x+1)%16][y][z] != 0) { // +x
              arrays.push_back (Vertex { v2f{1,0}, xcol * 0.7f, v3i {x+1, y+0, z+0} });
              arrays.push_back (Vertex { v2f{1,1}, xcol * 0.7f, v3i {x+1, y+0, z+1} });
              arrays.push_back (Vertex { v2f{0,1}, xcol * 0.7f, v3i {x+1, y+1, z+1} });
              arrays.push_back (Vertex { v2f{0,0}, xcol * 0.7f, v3i {x+1, y+1, z+0} });
            }
            if (yadj [x][(y+1)%16][z] != 0) { // +y
              arrays.push_back (Vertex { v2f{0,0}, ycol * 0.7f, v3i {x+0, y+1, z+0} });
              arrays.push_back (Vertex { v2f{1,0}, ycol * 0.7f, v3i {x+1, y+1, z+0} });
              arrays.push_back (Vertex { v2f{1,1}, ycol * 0.7f, v3i {x+1, y+1, z+1} });
              arrays.push_back (Vertex { v2f{0,1}, ycol * 0.7f, v3i {x+0, y+1, z+1} });
            }
            if (zadj [x][y][(z+1)%16] != 0) { // +z
              arrays.push_back (Vertex { v2f{1,1}, zcol * 0.7f, v3i {x+0, y+0, z+1} });
              arrays.push_back (Vertex { v2f{0,1}, zcol * 0.7f, v3i {x+0, y+1, z+1} });
              arrays.push_back (Vertex { v2f{0,0}, zcol * 0.7f, v3i {x+1, y+1, z+1} });
              arrays.push_back (Vertex { v2f{1,0}, zcol * 0.7f, v3i {x+1, y+0, z+1} });
            }
          }
        }
      }
    }
  }

  ChunkMesh::Shared ChunkMesh::create () {
    return std::make_shared<ChunkMeshImpl> ();
  }
}


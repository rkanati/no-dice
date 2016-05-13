//
// no-dice
//

#include "chunk-mesh.hpp"

#include <vector>

#include <GL/gl.h>

namespace nd {
  struct Vertex {
    float r, g, b, x, y, z;
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
    glColorPointer  (3, GL_FLOAT, 24, ((const char*) arrays.data ()) +  0);
    glVertexPointer (3, GL_FLOAT, 24, ((const char*) arrays.data ()) + 12);

    glDrawArrays (GL_QUADS, 0, arrays.size ());
  }

  void ChunkMeshImpl::regen (
    ChunkData const* chunk,
    Array<3, ChunkData const*> const& adjs)
  {
    arrays.clear ();

    for (int z = 0; z != 16; z++) {
      auto zadj = (z != 15)? chunk->blocks : adjs[2]->blocks;

      for (int y = 0; y != 16; y++) {
        auto yadj = (y != 15)? chunk->blocks : adjs[1]->blocks;

        for (int x = 0; x != 16; x++) {
          auto xadj = (x != 15)? chunk->blocks : adjs[0]->blocks;

          Block current = chunk->blocks [x][y][z];

          if (current != 0) { // face out
            if (xadj [(x+1)%16][y][z] == 0) { // +x
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+0.f, z+0.f });
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+1.f, z+0.f });
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+0.f, z+1.f });
            }
            if (yadj [x][(y+1)%16][z] == 0) { // +y
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+0.f, y+1.f, z+0.f });
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+0.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+1.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+1.f, y+1.f, z+0.f });
            }
            if (zadj [x][y][(z+1)%16] == 0) { // +z
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+0.f, y+0.f, z+1.f });
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+1.f, y+0.f, z+1.f });
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+1.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+0.f, y+1.f, z+1.f });
            }
          }
          else if (current == 0) { // face in
            if (xadj [(x+1)%16][y][z] != 0) { // +x
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+0.f, z+0.f });
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+0.f, z+1.f });
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 1.0f, 0.4f, 0.7f, x+1.f, y+1.f, z+0.f });
            }
            if (yadj [x][(y+1)%16][z] != 0) { // +y
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+0.f, y+1.f, z+0.f });
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+1.f, y+1.f, z+0.f });
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+1.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 0.2f, 0.7f, 0.5f, x+0.f, y+1.f, z+1.f });
            }
            if (zadj [x][y][(z+1)%16] != 0) { // +z
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+0.f, y+0.f, z+1.f });
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+0.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+1.f, y+1.f, z+1.f });
              arrays.push_back (Vertex { 0.0f, 0.0f, 0.8f, x+1.f, y+0.f, z+1.f });
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


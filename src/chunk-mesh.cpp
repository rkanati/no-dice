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

  struct ChunkMesh::Impl {
    std::vector <Vertex> arrays;
  };

  ChunkMesh::ChunkMesh (Impl* i) :
    impl (i)
  { }

  ChunkMesh::~ChunkMesh () {
    delete impl;
  }

  void ChunkMesh::draw () const {
    glColorPointer  (3, GL_FLOAT, 24, ((const char*) impl->arrays.data ()) +  0);
    glVertexPointer (3, GL_FLOAT, 24, ((const char*) impl->arrays.data ()) + 12);

    glDrawArrays (GL_QUADS, 0, impl->arrays.size ());
  }

  ChunkMesh ChunkMesh::generate (
    const Chunk& chunk,
    const Chunk& x_adj_chunk,
    const Chunk& y_adj_chunk,
    const Chunk& z_adj_chunk)
  {
    std::vector <Vertex> buf;
    buf.reserve (4 * 3 * 16 * 16 * 16);

    for (int z = 0; z != 16; z++) {
      auto zadj = (z != 15)? chunk.blocks : z_adj_chunk.blocks;

      for (int y = 0; y != 16; y++) {
        auto yadj = (y != 15)? chunk.blocks : y_adj_chunk.blocks;

        for (int x = 0; x != 16; x++) {
          auto xadj = (x != 15)? chunk.blocks : x_adj_chunk.blocks;

          Block current = chunk.blocks [x][y][z];

          if (current != 0) { // face out
            if (xadj [(x+1)%16][y][z] == 0) { // +x
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+0.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+0.f, z+1.f });
            }
            if (yadj [x][(y+1)%16][z] == 0) { // +y
              buf.push_back (Vertex { 0, 1, 0, x+0.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 0, 1, 0, x+0.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 0, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 0, x+1.f, y+1.f, z+0.f });
            }
            if (zadj [x][y][(z+1)%16] == 0) { // +z
              buf.push_back (Vertex { 0, 0, 1, x+0.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 0, 0, 1, x+1.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 0, 0, 1, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 0, 1, x+0.f, y+1.f, z+1.f });
            }
          }
          else if (current == 0) { // face in
            if (xadj [(x+1)%16][y][z] != 0) { // +x
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+0.f, z+0.f });
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+1.f, z+0.f });
            }
            if (yadj [x][(y+1)%16][z] != 0) { // +y
              buf.push_back (Vertex { 1, 0, 1, x+0.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 1, x+1.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 1, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 0, 1, x+0.f, y+1.f, z+1.f });
            }
            if (zadj [x][y][(z+1)%16] != 0) { // +z
              buf.push_back (Vertex { 1, 1, 0, x+0.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 1, 1, 0, x+0.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 1, 0, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 1, 0, x+1.f, y+0.f, z+1.f });
            }
          }
        }
      }
    }

    return new Impl { std::move (buf) };
  }
}


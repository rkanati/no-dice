//
// no-dice
//

#include "glcontext.hpp"
#include "xwin.hpp"

#include "types.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <stdexcept>
#include <vector>

#include <GL/gl.h>

namespace nd {
  struct Vertex {
    float r, g, b, x, y, z;
  };

  using Block = u8;

  struct Chunk {
    Block blocks [16][16][16];
  };

  class ChunkMesh {
    std::vector <Vertex> arrays;
    ChunkMesh (std::vector <Vertex> arrays_in) :
      arrays (std::move (arrays_in))
    { }

  public:
    static ChunkMesh generate (const Chunk&);
    void draw () const;
  };

  void ChunkMesh::draw () const {
    glColorPointer  (3, GL_FLOAT, 24, ((const char*) arrays.data ()) +  0);
    glVertexPointer (3, GL_FLOAT, 24, ((const char*) arrays.data ()) + 12);

    glDrawArrays (GL_QUADS, 0, arrays.size ());
  }

  ChunkMesh ChunkMesh::generate (const Chunk& chunk) {
    std::vector <Vertex> buf;
    buf.reserve (4 * 3 * 16 * 16 * 16);

    for (int z = 0; z != 15; z++) {
      for (int y = 0; y != 15; y++) {
        for (int x = 0; x != 15; x++) {
          Block current = chunk.blocks [x][y][z];

          if (current != 0) { // face out
            if (chunk.blocks [x+1][y][z] == 0) { // +x
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+0.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 0, 0, x+1.f, y+0.f, z+1.f });
            }
            if (chunk.blocks [x][y+1][z] == 0) { // +y
              buf.push_back (Vertex { 0, 1, 0, x+0.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 0, 1, 0, x+0.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 0, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 0, x+1.f, y+1.f, z+0.f });
            }
            if (chunk.blocks [x][y][z+1] == 0) { // +z
              buf.push_back (Vertex { 0, 0, 1, x+0.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 0, 0, 1, x+1.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 0, 0, 1, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 0, 1, x+0.f, y+1.f, z+1.f });
            }
          }
          else if (current == 0) { // face in
            if (chunk.blocks [x+1][y][z] != 0) { // +x
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+0.f, z+0.f });
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 0, 1, 1, x+1.f, y+1.f, z+0.f });
            }
            if (chunk.blocks [x][y+1][z] != 0) { // +y
              buf.push_back (Vertex { 1, 0, 1, x+0.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 1, x+1.f, y+1.f, z+0.f });
              buf.push_back (Vertex { 1, 0, 1, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 0, 1, x+0.f, y+1.f, z+1.f });
            }
            if (chunk.blocks [x][y][z+1] != 0) { // +z
              buf.push_back (Vertex { 1, 1, 0, x+0.f, y+0.f, z+1.f });
              buf.push_back (Vertex { 1, 1, 0, x+0.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 1, 0, x+1.f, y+1.f, z+1.f });
              buf.push_back (Vertex { 1, 1, 0, x+1.f, y+0.f, z+1.f });
            }
          }
        }
      }
    }

    return ChunkMesh { std::move (buf) };
  }

  void redraw (int width, int height, float t, const ChunkMesh& mesh) {
    glViewport (0, 0, width, height);

    glClearColor (0, 0, 0, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    float r = float (width) / float (height);
    float s = 20.0f;
    float x = r * s;
    float y = s;
    glOrtho (-x, x, -y, y, -s, s);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glRotatef (t * 100.0f, 0.5773f, 0.5773f, 0.5773f);

    mesh.draw ();
  }

  class Syncer {
    double prev_real_time;
    double game_time;
    double accumulator;

    static double now () {
      timespec ts = { 0 };
      clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
      return double (ts.tv_sec) + ts.tv_nsec * 0.000000001;
    }

  public:
    const double tick_hz = 50.0;
    const double time_step = 1.0 / tick_hz;

    Syncer () :
      prev_real_time (now ()),
      game_time      (0.0),
      accumulator    (0.0)
    { }

    void update () {
      double real_time = now ();
      accumulator += real_time - prev_real_time;
      prev_real_time = real_time;
    }

    bool need_tick () const {
      return accumulator >= time_step;
    }

    void begin_tick () {
      accumulator -= time_step;
      game_time   += time_step;
    }

    double time () const {
      return game_time;
    }

    float alpha () const {
      return accumulator * tick_hz;
    }

  };

  extern "C" int main () {
    Chunk chunk;
    for (int z = 0; z != 16; z++) {
      for (int y = 0; y != 16; y++) {
        for (int x = 0; x != 16; x++) {
          if (x < 2 || x > 13 || y < 2 || y > 13 || (z / 2) % 2)
            chunk.blocks [x][y][z] = 0;
          else
            chunk.blocks [x][y][z] = 1;
        }
      }
    }

    auto mesh = ChunkMesh::generate (chunk);

    auto host = XHost::create ();
    auto gl_ctx = GLContext::establish (host.egl_display (), host.egl_window ());

    glEnable (GL_CULL_FACE);
    glEnable (GL_DEPTH_TEST);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_COLOR_ARRAY);

    Syncer syncer;

    while (true) {
      // handle events
      auto input = host.pump ();
      if (input.quit)
        break;

      syncer.update ();
      while (syncer.need_tick ()) {
        syncer.begin_tick ();
        // ...
      }

      redraw (
        host.width (),
        host.height (),
        syncer.time () + syncer.time_step * syncer.alpha (),
        mesh
      );

      gl_ctx.flip ();
    }

  exit_loop:;
  }
}


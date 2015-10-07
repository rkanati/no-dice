//
// no-dice
//

#include "glcontext.hpp"
#include "xwin.hpp"

#include "types.hpp"

#include "chunk.hpp"
#include "chunk-mesh.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <stdexcept>
#include <vector>

#include <GL/gl.h>
#include <GL/glu.h>

namespace nd {
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

  void redraw (int width, int height, float t, const ChunkMesh& mesh) {
    if (width < 1) width = 1;
    if (height < 1) height = 1;

    glViewport (0, 0, width, height);

    glClearColor (0, 0, 0, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    double aspect = double (width) / double (height);
    gluPerspective (75.0 / aspect, aspect, 0.1, 100.0);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (
      8, 8, -24,
      8, 8, 8,
      0, 1, 0
    );

    glRotatef (t * 100.0f, 0.5773f, 0.5773f, 0.5773f);

    mesh.draw ();
  }

  extern "C" int main () {
    Chunk chunk;
    for (int z = 0; z != 16; z++) {
      for (int y = 0; y != 16; y++) {
        for (int x = 0; x != 16; x++) {
          int a = z/2, b = 15-z/2;
          if (x < a || x > b || y < a || y > b || (z) % 2)
            chunk.blocks [x][y][z] = 0;
          else
            chunk.blocks [x][y][z] = 1;
        }
      }
    }

    Chunk blank;
    for (int z = 0; z != 16; z++) {
      for (int y = 0; y != 16; y++) {
        for (int x = 0; x != 16; x++) {
          blank.blocks [x][y][z] = 0;
        }
      }
    }

    auto mesh = ChunkMesh::generate (chunk, blank, blank, blank);

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


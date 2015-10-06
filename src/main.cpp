//
// no-dice
//

#include "glcontext.hpp"
#include "xwin.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <stdexcept>

#include <GL/gl.h>

namespace nd {
  void redraw (int width, int height, float t) {
    glViewport (0, 0, width, height);

    glClearColor (0, 0, 0, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    float r = float (width) / float (height);
    float s = 5.0f;
    float x = r * s;
    float y = s;
    glOrtho (-x, x, -y, y, -10, 10);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glRotatef (t * 80.f, 0, 0, 1);
    glTranslatef (3, 0, 0);
    glRotatef (t * 100.0f, 0.5773f, 0.5773f, 0.5773f);

    float ch = 0.9, cm = 0.3, cl = 0.0;
    glBegin (GL_QUADS);
      glColor3f (ch, cm, cm);
      glVertex3f (1, -1, -1);
      glVertex3f (1,  1, -1);
      glVertex3f (1,  1,  1);
      glVertex3f (1, -1,  1);

      glColor3f (cm, cl, cl);
      glVertex3f (-1, -1, -1);
      glVertex3f (-1, -1,  1);
      glVertex3f (-1,  1,  1);
      glVertex3f (-1,  1, -1);

      glColor3f (cm, ch, cm);
      glVertex3f (-1, 1, -1);
      glVertex3f (-1, 1,  1);
      glVertex3f ( 1, 1,  1);
      glVertex3f ( 1, 1, -1);

      glColor3f (cl, cm, cl);
      glVertex3f (-1, -1, -1);
      glVertex3f ( 1, -1, -1);
      glVertex3f ( 1, -1,  1);
      glVertex3f (-1, -1,  1);

      glColor3f (cm, cm, ch);
      glVertex3f (-1, -1, 1);
      glVertex3f ( 1, -1, 1);
      glVertex3f ( 1,  1, 1);
      glVertex3f (-1,  1, 1);

      glColor3f (cl, cl, cm);
      glVertex3f (-1, -1, -1);
      glVertex3f (-1,  1, -1);
      glVertex3f ( 1,  1, -1);
      glVertex3f ( 1, -1, -1);
    glEnd ();
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
    auto host = XHost::create ();
    auto gl_ctx = GLContext::establish (host.egl_display (), host.egl_window ());

    glEnable (GL_DEPTH_TEST);

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

      redraw (host.width (), host.height (), syncer.time () + syncer.time_step * syncer.alpha ());
      gl_ctx.flip ();
    }

  exit_loop:;
  }
}


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

  float now () {
    timespec ts = { 0 };
    clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
    return double (ts.tv_sec) + ts.tv_nsec * 0.000000001;
  }

  extern "C" int main () {
    auto host = XHost::create ();
    auto gl_ctx = GLContext::establish (host.egl_display (), host.egl_window ());

    glEnable (GL_DEPTH_TEST);

    const float tick_hz = 50.0f;
    const float dt = 1.0f / tick_hz;

    float t = 0.0f;
    float accum_time = 0.0f;
    float prev_rt = now ();

    while (true) {
      // handle events
      auto input = host.pump ();
      if (input.quit)
        break;

      float cur_rt = now ();
      float diff_rt = cur_rt - prev_rt;
      accum_time += diff_rt;
      prev_rt = cur_rt;

      while (accum_time >= dt) {
        accum_time -= dt;
        t += dt;
      }

      redraw (host.width (), host.height (), t + accum_time);
      gl_ctx.flip ();
    }

  exit_loop:;
  }
}


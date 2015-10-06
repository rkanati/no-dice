//
// no-dice
//

#pragma once

#include <EGL/egl.h>

namespace nd {
  class GLContext {
    EGLDisplay disp;
    EGLContext ctx;
    EGLSurface surf;

    GLContext (EGLDisplay d, EGLContext c, EGLSurface s) :
      disp (d), ctx (c), surf (s)
    { }

  public:
    static GLContext establish (EGLNativeDisplayType, EGLNativeWindowType);
    ~GLContext ();
    void flip ();
  };
}


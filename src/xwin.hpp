//
// no-dice
//

#pragma once

#include <EGL/egl.h>

#include "input.hpp"

namespace nd {
  class XHost {
    struct Impl;
    Impl* impl;
    XHost (Impl*);

  public:
    static XHost create ();
    ~XHost ();

    InputFrame pump ();

    int width () const;
    int height () const;

    EGLNativeDisplayType egl_display ();
    EGLNativeWindowType  egl_window  ();
  };
}


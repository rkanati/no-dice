//
// no-dice
//

#pragma once

#include <EGL/egl.h>

#include "input.hpp"
#include "vector.hpp"

namespace nd {
  class XHost {
    class Impl;
    Impl* impl;
    XHost (Impl*);

  public:
    static XHost create ();
    ~XHost ();

    InputFrame pump ();

    int width () const;
    int height () const;

    v2i dims () const {
      return { width (), height () };
    }

    EGLNativeDisplayType egl_display ();
    EGLNativeWindowType  egl_window  ();
  };
}


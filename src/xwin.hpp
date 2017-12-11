//
// no-dice
//

#pragma once

#include <EGL/egl.h>

#include "input.hpp"
#include "vector.hpp"

namespace nd {
  class XHost {
  public:
    using Ptr = std::unique_ptr<XHost>;

    static Ptr create ();

    virtual InputFrame pump () = 0;

    virtual v2i dims () const = 0;

    int width  () const { return dims ().x; }
    int height () const { return dims ().y; }

    float aspect () const {
      v2f ds = dims ();
      return ds.x / ds.y;
    }

  //virtual auto egl_display () -> EGLNativeDisplayType = 0;
    virtual auto egl_window  () -> EGLNativeWindowType = 0;

    virtual auto keyboard () -> InputDevice* = 0;
    virtual auto pointer  () -> InputDevice* = 0;
  };
}


//
// no-dice
//

#pragma once

#include "placement.hpp"

#include <Rk/transform.hpp>

namespace nd {
  struct Frustum {
    m4f w2e, e2c, w2c;

    Frustum (Placement p, float vfov, float aspect) :
      w2e (Rk::world_to_eye (p.pos, p.ori)),
      e2c (Rk::eye_to_clip (vfov, aspect, 0.01f, 1000.0f)),
      w2c (e2c * w2e)
    { }

    bool intersects_cube (v3f mins, float side) const {
      for (v3i corner : vec_range (v3i{0,0,0}, v3i{2,2,2})) {
        v4f const
          pos = Rk::compose_vector (mins + side * corner, 1.f),
          clip = w2c * pos;
        if ( (clip.x >= -clip.w && clip.x <= clip.w)
          || (clip.y >= -clip.w && clip.y <= clip.w)
          || (clip.z >= -clip.w && clip.z <= clip.w))
        { return true; }
      }

      return false;
    }
  };
}


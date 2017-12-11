//
// no-dice
//

#pragma once

#include "vector.hpp"
#include "versor.hpp"

namespace nd {
  struct Placement {
    v3f   pos;
    versf ori;

    Placement () = default;

    Placement (Nil) :
      pos (nil), ori (identity)
    { }

    Placement (v3f pos, versf ori) :
      pos (pos), ori (ori)
    { }
  };

  static Placement lerp (Placement const& from, Placement const& to, float alpha) {
    return {
      lerp (from.pos, to.pos, alpha),
      lerp (from.ori, to.ori, alpha)
    };
  }
}


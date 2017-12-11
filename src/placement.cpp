//
// no-dice
//

#include "placement.hpp"

namespace nd {
  Placement lerp (Placement const& from, Placement const& to, float alpha) {
    return {
      lerp (from.pos, to.pos, alpha),
      lerp (from.ori, to.ori, alpha)
    };
  }
}


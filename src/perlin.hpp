//
// no-dice
//

#include "vector.hpp"

namespace nd {
  class CoordHasher {
    u8 perm[256];

  public:
    CoordHasher (u32 const seed);
    CoordHasher (CoordHasher const&) = delete;

    u8 hash_coords (v3i const v) const {
      u8 hx = perm [u8 (v.x)];
      u8 hy = perm [u8 (hx + v.y)];
      return perm [u8 (hy + v.z)];
    }

    u8 hash_coords (v2i const v) const {
      u8 hx = perm [u8 (v.x)];
      return perm [u8 (hx + v.y)];
    }

    template<uint n>
    u8 operator () (Rk::vector<n, int> coords) const {
      return hash_coords (coords);
    }
  };

  float perlin (CoordHasher const& hasher, v3f pos, int wrap = 0);
  float perlin (CoordHasher const& hasher, v2f pos, int wrap = 0);
}


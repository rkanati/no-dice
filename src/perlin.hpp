//
// no-dice
//

#include "vector.hpp"

namespace nd {
  class CoordHasher {
    u8 perm[256];

  public:
    CoordHasher (u32 const seed);

    u8 hash_coords (v3i const coords) const {
      auto v = Rk::vector3<u8> { (u8) coords.x, (u8) coords.y, (u8) coords.z };
      return perm[perm[perm[v.x] + v.y] + v.z];
    }

    u8 hash_coords (v2i const coords) const {
      auto v = Rk::vector2<u8> { (u8) coords.x, (u8) coords.y };
      return perm[perm[v.x] + v.y];
    }

    template<uint n>
    u8 operator () (Rk::vector<n, int> coords) const {
      return hash_coords (coords);
    }
  };

  float perlin (CoordHasher const& hasher, v3f pos);
  float perlin (CoordHasher const& hasher, v2f pos);
}


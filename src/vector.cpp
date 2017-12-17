//
// no-dice
//

#include "vector.hpp"

namespace nd {
  namespace {
    inline u32 rotl (u32 const x, i8 const r) {
      return (x << r) | (x >> (32 - r));
    }

    void step (u32& h, u32 v) {
      v *= 0xcc9e2d51;
      v = rotl (v, 15);
      v *= 0x1b873593;
      h ^= v;
      h = rotl (h, 13);
      h = h*5 + 0xe6546b64;
    }

    u32 done (u32 h) {
      h ^= h >> 16;
      h *= 0x85ebca6b;
      h ^= h >> 13;
      h *= 0xc2b2ae35;
      h ^= h >> 16;
      return h;
    }
  }

  u32 hash_coords (u32 h, int const v) {
    step (h, (u32) v);
    return done (h);
  }

  u32 hash_coords (u32 h, v2i const v) {
    step (h, (u32) v.x);
    step (h, (u32) v.y);
    return done (h);
  }

  u32 hash_coords (u32 h, v3i const v) {
    step (h, (u32) v.x);
    step (h, (u32) v.y);
    step (h, (u32) v.z);
    return done (h);
  }
}


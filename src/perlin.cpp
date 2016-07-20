//
// no-dice
//

#include "perlin.hpp"
#include "types.hpp"

#include <random>
#include <algorithm>
#include <functional>

namespace nd {
  CoordHasher::CoordHasher (u32 const seed) {
    std::mt19937 twister (seed);
    for (uint i = 0; i != 256; i++)
      perm[i] = i & 0xff;
    std::shuffle (perm, perm+256, twister);
  }

  namespace {
    v3f gradient (u8 hash) {
      static v3f const grads[16] = {
        v3f {  1,  1,  0 },
        v3f { -1,  1,  0 },
        v3f {  1, -1,  0 },
        v3f { -1, -1,  0 },
        v3f {  1,  0,  1 },
        v3f { -1,  0,  1 },
        v3f {  1,  0, -1 },
        v3f { -1,  0, -1 },
        v3f {  0,  1,  1 },
        v3f {  0, -1,  1 },
        v3f {  0,  1, -1 },
        v3f {  0, -1, -1 },
        v3f {  1,  1,  0 },
        v3f { -1,  0,  1 },
        v3f {  0, -1, -1 },
        v3f {  1,  0, -1 }
      };

      auto h = hash & 0xf;
      return grads[h];
    }

    auto grads_2d () -> Array<16, v2f> {
      Array<16, v2f> grads;
      float twopi = 6.28318530718f;
      float frac = 1.0f / 16.0f;
      for (uint i = 0; i != 16; i++) {
        float const theta = twopi * i * frac;
        grads[i] = v2f { std::cos (theta), std::sin (theta) };
      }
      return grads;
    }

    float contribution (u8 hash, v2f v) {
      static Array<16, v2f> const grads = grads_2d ();
      return dot (v, grads[hash & 0xf]);
    }

    float ease (float t) {
      return t * t * t * ((6 * t - 15) * t + 10);
    }
  }

  float perlin (CoordHasher const& hasher, v3f pos) {
    using Rk::lerp;

    // break into cell and relative (integer and fraction)
    v3i const cell = floor (pos);
    v3f const rel = pos - cell;

    // apply s-curve to smooth boundaries
    v3f const smoothing = transform (ease, rel),
              inv_smoothing = v3f{1,1,1} - smoothing;

    // offsets of cell corners
    static Rk::vector3<u8> const corners[8] = {
      v3i {0, 0, 0},
      v3i {0, 0, 1},
      v3i {0, 1, 0},
      v3i {0, 1, 1},
      v3i {1, 0, 0},
      v3i {1, 0, 1},
      v3i {1, 1, 0},
      v3i {1, 1, 1}
    };

    float value = 0.0f;
    for (auto i = 0; i != 8; i++) {
      v3i corner = corners[i],
          inv_corner = v3i{1,1,1} - corner;

      // smoothed contribution coefficient for this corner
      v3f coeffs = inv_corner * inv_smoothing + corner * smoothing;
      float coeff = coeffs.x * coeffs.y * coeffs.z;

      // get gradient for this corner
      u8 hash = hasher (cell + corner);
      v3f grad = gradient (hash);

      // add contribution
      float contrib = dot (grad, rel - corner);
      value += coeff * contrib;
    }

    return value;
  }

  float perlin (CoordHasher const& hasher, v2f pos) {
    using Rk::lerp;

    v2i const cell = floor (pos);
    v2f const rel  = pos - cell;

    auto const interp = transform (ease, rel);

    static v2i const offsets [4] = {
      v2i {0, 0},
      v2i {0, 1},
      v2i {1, 0},
      v2i {1, 1}
    };

    float contribs [4];
    for (uint i = 0; i != 4; i++)
      contribs[i] = contribution (hasher (cell + offsets[i]), rel - offsets[i]);

    for (uint i = 0; i != 2; i++)
      contribs[i*2] = lerp (contribs[i*2], contribs[i*2+1], interp.y);

    return lerp (contribs[0], contribs[2], interp.x);
  }
}


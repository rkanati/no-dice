//
// no-dice
//

#include "perlin.hpp"
#include "types.hpp"

#include <limits>
#include <random>
#include <utility>
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
    template<uint n>
    struct Constants;

    template<>
    struct Constants<3> {
      static constexpr auto grads () -> Array<16, v3f> {
        return {
          v3f { 1,  1,  0},
          v3f {-1,  1,  0},
          v3f { 1, -1,  0},
          v3f {-1, -1,  0},
          v3f { 1,  0,  1},
          v3f {-1,  0,  1},
          v3f { 1,  0, -1},
          v3f {-1,  0, -1},
          v3f { 0,  1,  1},
          v3f { 0, -1,  1},
          v3f { 0,  1, -1},
          v3f { 0, -1, -1},
          v3f { 1,  1,  0},
          v3f {-1,  0,  1},
          v3f { 0, -1, -1},
          v3f { 1,  0, -1}
        };
      }

      static constexpr auto corners () -> Array<8, v3i> {
        return {
          v3i {0, 0, 0},
          v3i {0, 0, 1},
          v3i {0, 1, 0},
          v3i {0, 1, 1},
          v3i {1, 0, 0},
          v3i {1, 0, 1},
          v3i {1, 1, 0},
          v3i {1, 1, 1}
        };
      }
    };

    template<int... is>
    constexpr auto grads_2d (std::integer_sequence<int, is...>) {
      float const twopi = 6.28318530718f;
      float const frac = 1.0f / 16.0f;
      return Array<sizeof... (is), v2f> {
        v2f { 1.414f * std::cos (twopi * is * frac), 1.414f * std::sin (twopi * is * frac) }
        ...
      };
    }

    template<>
    struct Constants<2> {
      static constexpr auto grads () {
        return grads_2d (std::make_integer_sequence<int, 16> { });
      }

      static constexpr auto corners () -> Array<4, v2i> {
        return {
          v2i {0, 0},
          v2i {0, 1},
          v2i {1, 0},
          v2i {1, 1}
        };
      }
    };

    template<uint i>
    auto gradient (u8 hash) {
      static constexpr auto const grads = Constants<i>::grads ();
      return grads[hash & 0xf];
    }

    float ease (float t) {
      return t * t * t * ((6 * t - 15) * t + 10);
    }

    template<uint n, typename T>
    vector<n, T> ones;

    template<typename T>
    auto ones<2, T> = vector2<T> {1,1};

    template<typename T>
    auto ones<3, T> = vector3<T> {1,1,1};

    template<uint n>
    float perlin_impl (CoordHasher const& hasher, vectorf<n> pos, int wrap) {
      if (wrap < 1)
        wrap = (std::numeric_limits<int>::max () / 2) + 1;

      // break into cell and relative (integer and fraction)
      vectori<n> cell = floor (pos);
      auto const rel = pos - cell;
      cell %= wrap;

      // apply s-curve to smooth boundaries
      auto const smoothing = transform (ease, rel),
                 inv_smoothing = ones<n, float> - smoothing;

      // offsets of cell corners
      static constexpr auto const corners = Constants<n>::corners ();

      float value = 0.0f;
      for (auto i = 0; i != corners.size (); i++) {
        auto corner = corners[i],
             inv_corner = ones<n, int> - corner;

        // smoothed contribution coefficient for this corner
        auto coeffs = inv_corner * inv_smoothing + corner * smoothing;
        float coeff = reduce (std::multiplies<> { }, coeffs);

        // get gradient for this corner
        u8 hash = hasher ((cell + corner) % wrap);
        auto grad = gradient<n> (hash);

        // add contribution
        float contrib = dot (grad, rel - corner);
        value += coeff * contrib;
      }

      return 0.5f + value * 0.5f;
    }
  }

  float perlin (CoordHasher const& hasher, v2f pos, int mask) {
    return perlin_impl (hasher, pos, mask);
  }

  float perlin (CoordHasher const& hasher, v3f pos, int mask) {
    return perlin_impl (hasher, pos, mask);
  }
}


//
// no-dice
//

#include "perlin.hpp"

#include "types.hpp"
#include "smooth.hpp"

#include <limits>
#include <random>
#include <utility>
#include <algorithm>
#include <functional>

namespace nd {
  namespace {
    template<uint n>
    struct Constants;

    template<>
    struct Constants<3> {
      static std::array<v3f, 16> constexpr grads {
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

      static std::array<v3i, 8> constexpr corners {
        v3i {0, 0, 0},
        v3i {0, 0, 1},
        v3i {0, 1, 0},
        v3i {0, 1, 1},
        v3i {1, 0, 0},
        v3i {1, 0, 1},
        v3i {1, 1, 0},
        v3i {1, 1, 1}
      };
    };

    template<int... is>
    auto constexpr grads_2d (std::integer_sequence<int, is...>) {
      float const
        twopi = 6.28318530718f,
        frac = 1.f/int (sizeof... (is)),
        k = twopi*frac;
      return std::array<v2f, sizeof... (is)> {
        1.414f * v2f { std::cos (k*is), std::sin (k*is) }
        ...
      };
    }

    template<>
    struct Constants<2> {
      static std::array<v2f, 16> constexpr grads
        = grads_2d (std::make_integer_sequence<int, 16> { });

      static std::array<v2i, 4> constexpr corners {
        v2i {0, 0},
        v2i {0, 1},
        v2i {1, 0},
        v2i {1, 1}
      };
    };

    template<uint i>
    auto gradient (u32 hash) {
      auto constexpr const grads = Constants<i>::grads;
      hash = (hash & 0xffff) ^ (hash >> 16);
      hash = (hash & 0x00ff) ^ (hash >>  8);
      hash = (hash & 0x000f) ^ (hash >>  4);
      return grads[hash];
    }

    template<uint n, typename T>
    vector<n, T> ones;

    template<typename T>
    auto ones<2, T> = vector2<T> {1,1};

    template<typename T>
    auto ones<3, T> = vector3<T> {1,1,1};

    template<uint n>
    float perlin_impl (VecHash hasher, vectorf<n> pos, int wrap) {
      if (wrap < 1)
        wrap = 1 << 31;

      // break into cell and relative (integer and fraction)
      vectori<n> cell = floor (pos);
      auto const rel = pos - cell;
      cell %= wrap;

      // apply s-curve to smooth boundaries
      auto const
        smoothing = transform (smoothstep<float>, rel),
        inv_smoothing = ones<n, float> - smoothing;

      // offsets of cell corners
      auto constexpr const corners = Constants<n>::corners;

      float value = 0.f;
      for (auto corner : corners) {
        auto const inv_corner = ones<n, int> - corner;

        // smoothed contribution coefficient for this corner
        auto const coeffs = inv_corner*inv_smoothing + corner*smoothing;
        float const coeff = reduce (std::multiplies<> { }, coeffs);

        // get gradient for this corner
        u32 const hash = hasher ((cell + corner) % wrap);
        auto const grad = gradient<n> (hash);

        // add contribution
        float const contrib = dot (grad, rel - corner);
        value += coeff*contrib;
      }

      // [0, 1]
      return 0.5f + 0.5f*value;
    }
  }

  float perlin (VecHash hasher, v2f pos, int wrap) {
    return perlin_impl (hasher, pos, wrap);
  }

  float perlin (VecHash hasher, v3f pos, int wrap) {
    return perlin_impl (hasher, pos, wrap);
  }
}


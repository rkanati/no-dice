//
// no-dice
//

#pragma once

#include <Rk/vector.hpp>

namespace nd {
  u32 hash_coords (u32 h, int const);
  u32 hash_coords (u32 h, v2i const);
  u32 hash_coords (u32 h, v3i const);

  class VecHash {
    u32 seed;

  public:
    explicit VecHash (u32 seed = 0) : seed (seed) { }

    template<typename V>
    std::size_t operator () (V v) const {
      return hash_coords (seed, v);
    }
  };

  namespace {
    template<typename OS, uint n, typename T>
    OS& operator << (OS& os, Rk::vector<n, T> v) {
      os << "(";
      for (int i = 0; i != n-1; i++)
        os << v[i] << " ";
      return os << v[n-1] << ")";
    }
  }
}


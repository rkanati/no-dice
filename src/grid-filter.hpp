//
// no-dice
//
#pragma once

#include "vector.hpp"
#include "smooth.hpp"

namespace nd {
  namespace detail {
    template<uint n, uint... idxs>
    auto constexpr make_corner_impl (int i, Rk::detail::idx_seq<idxs...>) {
      return Rk::make_vector (((i >> (n-idxs-1)) & 1)? 1 : 0 ...);
    }

    template<uint n>
    auto constexpr make_corner (int i) {
      return make_corner_impl<n> (i, vector<n, int>::zero_to_n ());
    }

    template<uint n, uint... idxs>
    auto constexpr make_corners_impl (Rk::detail::idx_seq<idxs...>)
      -> std::array<vectori<n>, (1<<n)>
    {
      return { make_corner<n> (idxs) ... };
    }
  }

  template<uint n>
  auto constexpr make_corners () {
    return detail::make_corners_impl<n> (Rk::detail::make_idxs<0,(1<<n)> ());
  }

  namespace detail {
    template<typename T>
    auto constexpr ipow (T x, uint p) {
      auto result = 1;
      while (p--)
        result *= x;
      return result;
    }

    template<uint... idxs>
    auto constexpr make_strides_impl (
      uint pitch, Rk::detail::idx_seq<idxs...>)
    {
      return Rk::make_vector (ipow (pitch, sizeof...(idxs)-idxs) ...);
    }

    template<uint n>
    auto constexpr make_strides (uint pitch) {
      return make_strides_impl (pitch, Rk::detail::make_idxs<1,n+1> ());
    }
  }

  template<int dim, uint n>
  class GridSamples {
    float samples[detail::ipow (dim+1, n)];

  public:
    template<typename Fn>
    static GridSamples take (vectorf<n> start, float step, Fn fn) {
      GridSamples gs;
      for (auto i : vec_range (Rk::diag<n> (0), Rk::diag<n> (dim+1)))
        gs[i] = fn (start + (step/dim)*i);
      return gs;
    }

    float operator [] (vectori<n> i) const {
      auto constexpr const strides = detail::make_strides<n> (dim+1);
      return samples[dot (i, strides)];
    }

    float& operator [] (vectori<n> i) {
      auto constexpr const strides = detail::make_strides<n> (dim+1);
      return samples[dot (i, strides)];
    }
  };

  namespace {
    template<int dim, uint n, typename Fn>
    inline GridSamples<dim, n> take_grid_samples (
      vectorf<n> start, float step, Fn fn)
    {
      return GridSamples<dim, n>::take (start, step, fn);
    }

    template<uint n, int dim, typename Interpolant>
    float grid_filter (
      GridSamples<dim, n> const& samples, Interpolant interp, vectorf<n> pos)
    {
      using Rk::lerp;

      pos *= dim;
      auto const cell = floor (pos);
      auto const smooth = transform (interp, pos - cell);

      auto constexpr const corners = make_corners<n> ();

      float scratch[corners.size ()];
      for (auto i = 0; i != corners.size (); i++)
        scratch[i] = samples[cell + corners[i]];

      for (uint p = n-1; p < n; p--) {
        for (uint i = 0; i != 1u<<p; i++)
          scratch[i] = lerp (scratch[2*i], scratch[2*i+1], smooth[p]);
      }

      return scratch[0];
    }

    template<uint n, int dim>
    float linear_filter (GridSamples<dim, n> const& samples, vectorf<n> pos) {
      auto id = [] (auto t) { return t; };
      return grid_filter (samples, id, pos);
    }

    template<uint n, int dim>
    float smooth_filter (GridSamples<dim, n> const& samples, vectorf<n> pos) {
      return grid_filter (samples, smoothstep<float>, pos);
    }
  }
}


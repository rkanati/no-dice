//
// no-dice
//
#pragma once

#include "vector.hpp"
#include "smooth.hpp"

namespace nd {
  namespace detail {
    template<uint n, uint... idxs>
    constexpr auto make_corner_impl (int i, Rk::detail::idx_seq<idxs...>) {
      return Rk::make_vector (((i >> (n-idxs-1)) & 1)? 1 : 0 ...);
    }

    template<uint n>
    constexpr auto make_corner (int i) {
      return make_corner_impl<n> (i, Rk::vector<n, int>::zero_to_n ());
    }

    template<uint n, uint... idxs>
    constexpr auto make_corners_impl (Rk::detail::idx_seq<idxs...>) -> Array<(1<<n), vectori<n>> {
      return { make_corner<n> (idxs) ... };
    }
  }

  template<uint n>
  constexpr auto make_corners () -> Array<(1<<n), vectori<n>> {
    return detail::make_corners_impl<n> (Rk::detail::make_idxs<0,(1<<n)> ());
  }

  template<uint n, typename T, typename Ifn>
  class GridFilter {
    Ifn interpolant;

  public:
    GridFilter (Ifn interpolant) :
      interpolant (interpolant)
    { }

  };

  namespace detail {
    template<typename T>
    constexpr auto ipow (T x, uint p) {
      auto result = 1;
      while (p--)
        result *= x;
      return result;
    }

    template<uint... idxs>
    constexpr auto make_strides_impl (uint pitch, Rk::detail::idx_seq<idxs...>) {
      return Rk::make_vector (ipow (pitch, sizeof...(idxs)-idxs) ...);
    }

    template<uint n>
    constexpr auto make_strides (uint pitch) {
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
      for (auto i : vec_range (Rk::zero_vector<n, int>, Rk::diag<n> (dim+1)))
        gs[i] = fn (start + (step/dim)*i);
      return gs;
    }

    float operator [] (vectori<n> i) const {
      static constexpr auto const strides = detail::make_strides<n> (dim+1);
      return samples[dot (i, strides)];
    }

    float& operator [] (vectori<n> i) {
      static constexpr auto const strides = detail::make_strides<n> (dim+1);
      return samples[dot (i, strides)];
    }
  };

  template<int dim, uint n, typename Fn>
  static inline GridSamples<dim, n> take_grid_samples (vectorf<n> start, float step, Fn fn) {
    return GridSamples<dim, n>::take (start, step, fn);
  }

  template<uint n, int dim, typename Interpolant>
  float grid_filter (GridSamples<dim, n> const& samples, Interpolant interp, vectorf<n> pos) {
    using Rk::lerp;

    pos *= dim;
    vectori<n> const cell = floor (pos);
    vectorf<n> const smooth = transform (interp, pos - cell);

    static constexpr auto const corners = make_corners<n> ();

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
    return grid_filter (samples, smoothstep, pos);
  }
}


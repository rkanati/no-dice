//
// no-dice
//

#pragma once

#include "vector.hpp"

namespace nd {
  template<unsigned n, typename X>
  class VecIter {
  public:
    using value_type = Rk::vector<n, X>;

  private:
    value_type  v;
    const value_type begin;
    const value_type end;

  public:
    VecIter (value_type new_begin, value_type new_end) :
      v     (new_begin),
      begin (new_begin),
      end   (new_end)
    { }

    VecIter () :
      v     (nil),
      begin (nil),
      end   (nil)
    { }

    explicit operator bool () const {
      return v != end;
    }

    VecIter operator ++ () {
      for (int i = 0; i != n; i++) {
        v[i]++;
        if (v[i] != end[i])
          break;
        v[i] = begin[i];
      }
      if (v == begin)
        v = end;
      return *this;
    }

    VecIter operator ++ (int) {
      auto old = (*this);
      ++(*this);
      return old;
    }

    bool operator == (VecIter const& other) const {
      return !*this && !other;
    }

    bool operator != (VecIter const& other) const {
      return !!other || !!*this;
    }

    auto operator * () const {
      return v;
    }
  };

  template<unsigned n, typename X>
  class VecRange {
  public:
    using iterator = VecIter<n, X>;

  private:
    Rk::vector<n, X> mins, maxs;

  public:
    constexpr explicit VecRange (Rk::vector<n, X> mins, Rk::vector<n, X> maxs) :
      mins (mins), maxs (maxs)
    { }

    iterator begin () {
      return { mins, maxs };
    }

    iterator end () {
      return { };
    }
  };

  template<unsigned n, typename X>
  static auto constexpr vec_range (Rk::vector<n, X> begin, Rk::vector<n, X> end) {
    return VecRange<n, X> (begin, end);
  }
}


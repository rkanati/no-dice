//
// no-dice
//

#pragma once

#include "vector.hpp"

namespace nd {
  template<unsigned n, typename Component>
  class VecIter {
  public:
    typedef Rk::vector<n, Component> Vector;

  private:
    Vector v;
    const Vector begin;
    const Vector end;

  public:
    VecIter (Vector new_begin, Vector new_end) :
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

    Vector vec () const {
      return v;
    }
  };

  template <unsigned n, typename Component>
  VecIter <n, Component> vec_iter (Rk::vector <n, Component> begin, Rk::vector <n, Component> end) {
    return VecIter <n, Component> (begin, end);
  }
}


//
// no-dice
//

#pragma once

#include "vector.hpp"

namespace nd {
  template<unsigned n, typename X>
  class VecIter {
  public:
    using value_type = vector<n, X>;

  private:
    value_type cur;
    value_type const begin, end;

  public:
    VecIter (value_type new_begin, value_type new_end) :
      cur   (new_begin),
      begin (new_begin),
      end   (new_end)
    { }

    VecIter () :
      cur   (nil),
      begin (nil),
      end   (nil)
    { }

    explicit operator bool () const {
      return cur != end;
    }

    VecIter operator ++ () {
      for (int i = n-1; i >= 0; i--) {
        cur[i]++;
        if (cur[i] != end[i])
          break;
        cur[i] = begin[i];
      }
      if (cur == begin)
        cur = end;
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
      return cur;
    }
  };

  template<unsigned n, typename X>
  class VecRange {
  public:
    using iterator = VecIter<n, X>;

  private:
    vector<n, X> mins, maxs;

  public:
    explicit constexpr VecRange (vector<n, X> mins, vector<n, X> maxs) :
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
  static auto constexpr vec_range (vector<n, X> begin, vector<n, X> end) {
    return VecRange<n, X> (begin, end);
  }
}


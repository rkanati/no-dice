//
// no-dice
//

#pragma once

#include <Rk/vector.hpp>

#include <vector>

namespace Rk {
  class Rect {
    v2i tl, br;

    Rect (v2i tl, v2i br) :
      tl (tl), br (br)
    { }

  public:
    Rect () : tl{0,0}, br{0,0} { }

    Rect (Rect const&) = default;

    static Rect with_corners_u (v2i tl, v2i br) {
      return Rect (tl, br);
    }

    static Rect with_corners (v2i a, v2i b) {
      v2i const
        tl { std::min (a.x, b.x), std::min (a.y, b.y) },
        br { std::max (a.x, b.x), std::max (a.y, b.y) };
      return Rect (tl, br);
    }

    static Rect with_dims (v2i p, v2i wh) {
      return with_corners (p, p+wh);
    }

    static Rect with_dims_u (v2i tl, v2i wh) {
      return with_corners_u (tl, tl+wh);
    }

    int min_side () const { return min (dims ()); }
    int max_side () const { return max (dims ()); }

    int width  () const { return xmax () - xmin (); }
    int height () const { return ymax () - ymin (); }
    v2i dims   () const { return maxs () - mins (); }

    int xmin () const { return tl.x; }
    int ymin () const { return tl.y; }
    v2i mins () const { return tl; }

    int xmax () const { return br.x; }
    int ymax () const { return br.y; }
    v2i maxs () const { return br; }

    bool covers (Rect const& other) const {
      return other.xmin () >= xmin ()
          && other.xmax () <= xmax ()
          && other.ymin () >= ymin ()
          && other.ymax () <= ymax ();
    }

    explicit operator bool () const {
      return dims () != v2i{0,0};
    }
  };

  class RectPacker {
    std::vector<Rect> free;

  public:
    explicit RectPacker (v2i dims, size_t reserve = 1) {
      free.reserve (reserve);
      free.push_back (Rect::with_dims (v2i{0,0}, dims));
    }

    struct AddResult {
      bool ok;
      bool rotated;
      Rect where;
    };

    AddResult add (v2i dims);
  };
}


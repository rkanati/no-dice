//
// no-dice
//

#include "bin-pack.hpp"

namespace Rk {
  namespace {
    inline bool orient (v2i dims) {
      return dims.y > dims.x;
    }
  }

  // MAXRECTS-BSSF
  RectPacker::AddResult RectPacker::add (v2i dims) {
    if (dims.x == 0 || dims.y == 0)
      return { true, false, Rect { } };

    // find the best place to put the rect
    std::vector<Rect>::iterator best_iter = free.end ();
    int best_waste = std::numeric_limits<int>::max ();

    for (auto
      free_iter  = free.begin ();
      free_iter != free.end ();
      free_iter++)
    {
      Rect const& cand = *free_iter;
      if (dims.x > cand.width () || dims.y > cand.height ())
        continue;
      // score by tightest short-side fit
      int const waste = cand.min_side () - min (dims);
      if (waste < best_waste) {
        best_waste = waste;
        best_iter = free_iter;
      }
    }

    // won't fit in this bin
    if (best_iter == free.end ())
      return { false, false, Rect { } };

    // rotate as necessary
    Rect const dest = *best_iter;
    bool const rotated = orient (dims) != orient (dest.dims ());
    if (rotated)
      std::swap (dims.x, dims.y);

    // add allocation
    Rect const packed = Rect::with_dims (dest.mins (), dims);

    // split free rect into overlapping pair
    Rect const
      a = Rect::with_corners (dest.mins () + v2i{dims.x,0}, dest.maxs ()),
      b = Rect::with_corners (dest.mins () + v2i{0,dims.y}, dest.maxs ());

    // replace free rect with one, add other to list
    if (a) {
      *best_iter = a;
      if (b)
        free.push_back (b);
    }
    else if (b) {
      *best_iter = b;
    }

    // exclude allocation from free rects
    for (size_t
      i_free = 0, limit = free.size ();
      i_free != limit;
      i_free++)
    {
      Rect const& cur = free[i_free];
      Rect substs[4];
      int n_substs = 0;

      // left rect needed?
      if (packed.xmin () > cur.xmin ()) {
        substs[n_substs++] = Rect::with_corners (
          cur.mins (),
          v2i { std::min (packed.xmin (), cur.xmax ()), cur.ymax () }
        );
      }

      // right rect needed?
      if (packed.xmax () < cur.xmax ()) {
        substs[n_substs++] = Rect::with_corners (
          v2i { std::max (packed.xmax (), cur.xmin ()), cur.ymin () },
          cur.maxs ()
        );
      }

      // top rect needed?
      if (packed.ymin () > cur.ymin ()) {
        substs[n_substs++] = Rect::with_corners (
          cur.mins (),
          v2i { cur.xmax (), std::min (packed.ymin (), cur.ymax ()) }
        );
      }

      // bottom rect needed?
      if (packed.ymax () < cur.ymax ()) {
        substs[n_substs++] = Rect::with_corners (
          v2i { cur.xmin (), std::max (packed.ymax (), cur.ymin ()) },
          cur.maxs ()
        );
      }

      if (n_substs > 0) {
        free[i_free] = substs[0];
        for (int i = 1; i != n_substs; i++)
          free.push_back (substs[i]);
      }
    }

    // remove redundant rects
    if (!free.empty ()) {
      std::vector<Rect> new_free;
      new_free.reserve (free.size ());

      // loop invariant: ∀r,s ∈ new_free. r⊈s
      for (size_t i = 0; i != free.size (); i++) {
        Rect const& r = free[i];
        if (!r)
          continue;

        bool add = true;

        for (auto iter = new_free.begin (); add && iter != new_free.end ();) {
          Rect const& s = *iter;
          if (r.covers (s))
            iter = new_free.erase (iter);
          else if (s.covers (r))
            add = false;
          else
            iter++;
        }

        if (add)
          new_free.push_back (r);
      }

      free = std::move (new_free);
    }

    return { true, rotated, packed };
  }

}


//
// no-dice
//

#pragma once

#include "vector.hpp"
#include "vec-iter.hpp"
#include "maybe.hpp"
#include "types.hpp"
#include "stage-chunk.hpp"

#include <vector>

namespace nd {
  class Stage {
  private:
    int const         dim;
    StageChunk* const chunks;
    vec3i             centre;

    vec3i abs_for_rel (vec3i rel) const {
      return rel + centre;
    }

    int index_abs (vec3i pos) const;
    int index_rel (vec3i rel) const;

    StageChunk&       chunk_at (int index);
    StageChunk const& chunk_at (int index) const;

  public:
    Stage (unsigned radius);
    ~Stage ();

    unsigned size () const {
      return dim * dim * dim;
    }

    vec3i maxs () const {
      const int r = dim / 2;
      return { r, r, r };
    }

    vec3i mins () const {
      return -maxs () + v3i{1,1,1};
    }

    auto indices () const {
      return vec_range (mins (), maxs () + v3i{1,1,1});
    }

    void relocate (vec3i new_centre, std::vector<vec3i>& missing);

    template<typename Pred>
    void find_all_if (Pred pred, std::vector<vec3i>& list) {
      list.clear ();

      for (auto rel : indices ()) {
        auto chunk = at_relative (rel);
        if (!chunk)
          continue;

        if (pred (*chunk))
          list.push_back (abs_for_rel (rel));
      }
    }

    void insert (StageChunk chunk);

    auto at_absolute (vec3i pos) const -> StageChunk const*;
    auto at_relative (vec3i off) const -> StageChunk const*;

    auto at_absolute (vec3i pos) -> StageChunk*;
    auto at_relative (vec3i off) -> StageChunk*;
  };
}


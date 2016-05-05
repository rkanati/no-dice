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

    StageChunk& chunk_at (int index);
    StageChunk const& chunk_at (int index) const;

  public:
    Stage (unsigned radius);
    ~Stage ();

    void relocate (vec3i new_centre, std::vector<vec3i>& missing);

    template<typename Pred>
    void find_all_if (Pred pred, std::vector<vec3i>& list) {
      list.clear ();

      for (auto rel = indexer (); rel; rel++) {
        auto chunk = at_relative (rel.vec ());
        if (!chunk)
          continue;

        if (pred (*chunk))
          list.push_back (abs_for_rel (rel.vec ()));
      }
    }

    void insert (StageChunk chunk, vec3i pos);

    auto at_absolute (vec3i pos) const -> StageChunk const*;
    auto at_relative (vec3i off) const -> StageChunk const*;

    auto at_absolute (vec3i pos) -> StageChunk*;
    auto at_relative (vec3i off) -> StageChunk*;

    unsigned size () const {
      return dim * dim * dim;
    }

    vec3i maxs () const {
      const int r = dim / 2;
      return { r, r, r };
    }

    vec3i mins () const {
      return -maxs ();
    }

    auto indexer () const -> VecIter<3, int> {
      return vec_iter (mins (), maxs ());
    }
  };
}


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
    v3i const         strides;
    StageChunk* const chunks;
    vec3i             centre;

    vec3i abs_for_rel (vec3i rel) const {
      return rel + centre;
    }

    int index_abs (vec3i pos) const {
      auto ijk = (pos % dim + v3i{dim,dim,dim}) % dim;
      // if (ijk.x < 0) ijk.x += dim;
      // if (ijk.y < 0) ijk.y += dim;
      // if (ijk.z < 0) ijk.z += dim;
      return dot (ijk, strides);
    }

    int index_rel (vec3i rel) const {
      return index_abs (abs_for_rel (rel));
    }

    StageChunk& chunk_at (int index) {
      return chunks [index];
    }

    StageChunk const& chunk_at (int index) const {
      return chunks [index];
    }

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

    void insert (StageChunk chunk) {
      auto& element = chunk_at (index_abs (chunk.position));
      element = std::move (chunk);
    }

    auto at_absolute (vec3i pos) const -> StageChunk const* {
      auto const& cell = chunk_at (index_abs (pos));
      if (!cell || cell.position != pos) return nullptr;
      else return &cell;
    }

    auto at_relative (vec3i off) const -> StageChunk const* {
      return at_absolute (abs_for_rel (off));
    }

    auto at_absolute (vec3i pos) -> StageChunk* {
      auto& cell = chunk_at (index_abs (pos));
      if (!cell || cell.position != pos) return nullptr;
      else return &cell;
    }

    auto at_relative (vec3i off) -> StageChunk* {
      return at_absolute (abs_for_rel (off));
    }
  };
}


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

    int index_abs (vec3i pos) const;
    int index_rel (vec3i rel) const;

    StageChunk& chunk_at (int index);
    StageChunk const& chunk_at (int index) const;

  public:
    Stage (unsigned radius);
    ~Stage ();

    auto relocate (vec3i new_centre, std::vector<vec3i> missing = { })
      -> std::vector<vec3i>;

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

    auto indexer () const {
      return vec_iter (mins (), maxs ());
    }
  };
}


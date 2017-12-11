//
// no-dice
//

#pragma once

#include "vector.hpp"
#include "vec-iter.hpp"
#include "types.hpp"
#include "stage-chunk.hpp"

#include <vector>

namespace nd {
  class Stage {
  private:
    int const         dim;
    v3i const         strides;
    StageChunk* const chunks;
    v3i               centre;

    int index_abs (v3i pos) const {
      auto ijk = (pos % dim + v3i{dim,dim,dim}) % dim;
      return dot (ijk, strides);
    }

    int index_rel (v3i rel) const {
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

    v3i maxs () const {
      int const r = dim / 2;
      return { r, r, r };
    }

    v3i mins () const {
      return -maxs () + v3i{1,1,1};
    }

    auto indices () const {
      return vec_range (mins (), maxs () + v3i{1,1,1});
    }

    bool in_radius (v3i rel) const {
      using namespace Rk::swiz;
      int const r = dim / 2;
      return abs2 (rel(X,Y)) <= r*r;
    }

    v3i abs_for_rel (v3i rel) const {
      return rel + centre;
    }

    auto relocate (v3i new_centre, std::vector<v3i>& missing)
      -> std::vector<v3i>&;

    auto remesh (std::vector<v3i>& to_mesh)
      -> std::vector<v3i>&;

    void update_data (ChunkData::Shared data, v3i pos) {
      at_absolute (pos)->update_data (std::move (data), pos);
    }

    void update_mesh (ChunkMesh mesh, v3i pos) {
      at_absolute (pos)->update_mesh (std::move (mesh), pos);
    }

    void cancel_meshing (v3i pos) {
      at_absolute (pos)->cancel_meshing ();
    }

    auto at_absolute (v3i pos) const -> StageChunk const* {
      return &chunk_at (index_abs (pos));
    }

    auto at_relative (v3i off) const -> StageChunk const* {
      return &chunk_at (index_rel (off));
    }

    auto at_absolute (v3i pos) -> StageChunk* {
      return &chunk_at (index_abs (pos));
    }

    auto at_relative (v3i off) -> StageChunk* {
      return &chunk_at (index_rel (off));
    }
  };
}


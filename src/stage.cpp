//
// no-dice
//

#include "stage.hpp"

namespace nd {
  int Stage::index_abs (vec3i pos) const {
    auto ijk = pos % dim;
    if (ijk.x < 0) ijk.x += dim;
    if (ijk.y < 0) ijk.y += dim;
    if (ijk.z < 0) ijk.z += dim;
    auto strides = vec3i { (dim * dim), dim, 1 };
    return dot (ijk, strides);
  }

  int Stage::index_rel (vec3i rel) const {
    return index_abs (abs_for_rel (rel));
  }

  StageChunk& Stage::chunk_at (int index) {
    return chunks [index];
  }

  StageChunk const& Stage::chunk_at (int index) const {
    return chunks [index];
  }

  Stage::Stage (unsigned radius) :
    dim (radius * 2),
    chunks (new StageChunk [dim * dim * dim]),
    centre {0,0,0}
  { }

  Stage::~Stage () {
    delete[] chunks;
  }

  void Stage::relocate (vec3i new_centre, std::vector<vec3i>& missing) {
    centre = new_centre;

    missing.clear ();
    for (auto rel : indices ()) {
      vec3i correct_pos = abs_for_rel (rel);
      auto& chunk = chunk_at (index_rel (rel));
      if (!chunk || chunk.position != correct_pos)
        missing.push_back (correct_pos);
    }
  }

  void Stage::insert (StageChunk chunk, vec3i pos) {
    auto& element = chunk_at (index_abs (pos));
    element = std::move (chunk);
  }

  auto Stage::at_absolute (vec3i pos) const -> StageChunk const* {
    auto const& cell = chunk_at (index_abs (pos));
    if (!cell || cell.position != pos) return nullptr;
    else return &cell;
  }

  auto Stage::at_absolute (vec3i pos) -> StageChunk* {
    auto& cell = chunk_at (index_abs (pos));
    if (!cell || cell.position != pos) return nullptr;
    else return &cell;
  }

  auto Stage::at_relative (vec3i off) const -> StageChunk const* {
    return at_absolute (abs_for_rel (off));
  }

  auto Stage::at_relative (vec3i off) -> StageChunk* {
    return at_absolute (abs_for_rel (off));
  }
}


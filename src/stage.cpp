//
// no-dice
//

#include "stage.hpp"

namespace nd {
  Stage::Stage (unsigned radius) :
    dim (radius * 2),
    strides { dim*dim, dim, 1 },
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
      auto& chunk = chunk_at (index_abs (correct_pos));
      if (!chunk || chunk.position != correct_pos)
        missing.push_back (correct_pos);
    }
  }
}


//
// no-dice
//

#include "stage.hpp"

#include <iostream>

namespace nd {
  Stage::Stage (unsigned radius) :
    dim ((radius*2 + chunk_dim - 1) / chunk_dim),
    strides { dim*dim, dim, 1 },
    chunks (new StageChunk [dim * dim * dim]),
    centre {0,0,0}
  { }

  Stage::~Stage () {
    delete[] chunks;
  }

  auto Stage::relocate (v3i new_centre, std::vector<v3i>& missing)
    -> std::vector<v3i>&
  {
    missing.clear ();
    centre = new_centre;

    for (auto rel : indices ()) {
      if (!in_radius (rel))
        continue;

      v3i correct_pos = abs_for_rel (rel);
      auto& chunk = *at_absolute (correct_pos);
      if (chunk.need_data (correct_pos))
        missing.push_back (correct_pos);
    }

    return missing;
  }

  auto Stage::remesh (std::vector<v3i>& to_mesh)
    -> std::vector<v3i>&
  {
    to_mesh.clear ();

    for (auto rel : indices ()) {
      if (!in_radius (rel))
        continue;

      v3i pos = abs_for_rel (rel);
      auto& chunk = *at_absolute (pos);
      if (chunk.need_mesh ())
        to_mesh.push_back (pos);
    }

    return to_mesh;
  }
}


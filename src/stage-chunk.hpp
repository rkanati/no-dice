//
// no-dice
//

#pragma once

#include "chunk.hpp"
#include "chunk-mesh.hpp"
#include "types.hpp"

#include <iostream>

namespace nd {
  class StageChunk {
    enum State {
      dead         = 0,
      pending_data = 1,
      loaded       = 2,
      pending_mesh = 3,
      meshed       = 4
    } state = dead;

    Shared<ChunkData> data;
    ChunkMesh         mesh;
    v3i               position;

  public:
    StageChunk () = default;

    StageChunk (StageChunk const&) = delete;
    StageChunk (StageChunk&&) = delete;
    StageChunk& operator = (StageChunk const&) = delete;
    StageChunk& operator = (StageChunk&&) = delete;

    bool need_data (v3i new_pos) {
      if (state != dead && new_pos == position)
        return false;

      state = pending_data;
      data = nullptr;
      mesh = nullptr;
      position = new_pos;
      return true;
    }

    bool need_mesh () {
      if (state != loaded)
        return false;

      state = pending_mesh;
      return true;
    }

    void cancel_meshing () {
      if (state == pending_mesh)
        state = loaded;
    }

    Shared<ChunkData> get_data (v3i want_pos) const {
      if (state >= loaded && position == want_pos) return data;
      else return nullptr;
    }

    ChunkMesh get_mesh (v3i want_pos) const {
      if (state == meshed && position == want_pos) return mesh;
      else return nullptr;
    }

    void update_data (Shared<ChunkData> new_data, v3i data_pos) {
      if (!new_data || state != pending_data || data_pos != position) {
        std::cerr << "Rejecting data " << position << "<-" << data_pos << "\n";
        return;
      }

      data = std::move (new_data);
      state = loaded;
    }

    void update_mesh (ChunkMesh new_mesh, v3i mesh_pos) {
      if (state != pending_mesh || mesh_pos != position) {
        std::cerr << "Rejecting mesh " << position << "<-" << mesh_pos << "\n";
        return;
      }

      mesh = std::move (new_mesh);
      state = meshed;
    }
  };
}


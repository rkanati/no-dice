//
// no-dice
//

#include "glcontext.hpp"
#include "xwin.hpp"

#include "types.hpp"

#include "frame.hpp"
#include "chunk.hpp"
#include "chunk-mesh.hpp"
#include "stage.hpp"
#include "syncer.hpp"
#include "stage-chunk.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <array>

#include <GL/gl.h>
#include <GL/glu.h>

namespace nd {
  void configure_gl () {
    glEnable (GL_CULL_FACE);
    glEnable (GL_DEPTH_TEST);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_COLOR_ARRAY);
  }

  auto make_test_chunkdata () -> ChunkData {
    ChunkData chunk;
    for (int z = 0; z != 16; z++) {
      for (int y = 0; y != 16; y++) {
        for (int x = 0; x != 16; x++) {
          int a = z/2, b = 15-z/2;
          if (x < a || x > b || y < a || y > b || (z) % 2)
            chunk.blocks [x][y][z] = 0;
          else
            chunk.blocks [x][y][z] = 1;
        }
      }
    }
    return chunk;
  }

  auto make_blank_chunkdata () -> ChunkData {
    ChunkData blank;
    for (int z = 0; z != 16; z++) {
      for (int y = 0; y != 16; y++) {
        for (int x = 0; x != 16; x++) {
          blank.blocks [x][y][z] = 0;
        }
      }
    }
    return blank;
  }

  auto get_cdata (vec3i pos) -> ChunkData {
    static auto const test_cdata = make_test_chunkdata ();
    static auto const blank_cdata = make_blank_chunkdata ();

    if (pos == vec3i{0,0,0}) return test_cdata;
    else return blank_cdata;
  }

  auto load_chunk (vec3i pos) -> StageChunk {
    auto cdata = share (get_cdata (pos));
    assert (!!cdata);
    return StageChunk (std::move (cdata), pos);
  }

  bool get_adjacent_chunk_datas (
    Array<3, ChunkData const*>& adjs,
    Stage const& stage,
    vec3i const pos)
  {
    static auto const offsets = Array<3, vec3i> {{ v3i{1,0,0}, v3i{0,1,0}, v3i{0,0,1} }};
    for (auto i = 0; i != 3; i++) {
      auto adj_chunk = stage.at_relative (pos + offsets[i]);
      if (!adj_chunk) return false;

      auto const adj_data = adj_chunk->data.get ();
      if (!adj_data) return false;
      else adjs[i] = adj_data;
    }

    return true;
  }

  template<typename OS, uint n, typename T>
  OS& operator << (OS& os, Rk::vector<n, T> v) {
    os << "( ";
    for (auto x : v)
      os << x << " ";
    return os << ")";
  }

  extern "C" int main () try {
    // subsystems
    auto host = XHost::create ();
    auto gl_ctx = GLContext::establish (host.egl_display (), host.egl_window ());
    configure_gl ();

    // persistent state
    Syncer syncer;

    Stage stage (3);
    vec3i player_pos {0,0,0};

    // transient state
    std::vector<vec3i> required_chunks;
    Frame frame;

    // main loop
    while (true) {
      // handle events
      auto input = host.pump ();
      if (input.quit)
        break;

      // advance simulation
      syncer.update ();
      while (syncer.need_tick ()) {
        syncer.begin_tick ();
        // ...
      }

      // update stage
      required_chunks = stage.relocate (player_pos, std::move (required_chunks));
      for (vec3i chunk_pos : required_chunks) {
        // load or generate chunk at chunk_pos
        std::cerr << "generating chunk for " << chunk_pos << "... ";
        auto chunk = load_chunk (chunk_pos);

        stage.insert (std::move (chunk), chunk_pos);
        std::cerr << "done\n";
      }

      // generate meshes
      for (auto idx = stage.indexer (); idx; idx++) {
        auto chunk = stage.at_relative (idx.vec ());

        // Skip meshgen for unloaded chunks or chunks with meshes
        if (!chunk || chunk->mesh)
          continue;

        // get +ve adjacent chunk data for correct meshgen
        // chunks on the +ve boundary of the stage naturally have no adjacencies
        Array<3, ChunkData const*> adjs;
        if (!get_adjacent_chunk_datas (adjs, stage, idx.vec ()))
          continue;

        std::cerr << "generating mesh for " << idx.vec () << "... ";
        chunk->mesh = ChunkMesh::generate (chunk->data.get (), adjs);
        if (!chunk->mesh)
          std::cerr << "failed\n";

        std::cerr << "ok\n";
      }

      // redraw
      for (auto idx = stage.indexer (); idx; idx++) {
        auto chunk = stage.at_relative (idx.vec ());
        if (!chunk || !chunk->mesh)
          continue;

        frame.add_cmesh (chunk->mesh.get ());
      }

      frame = frame.draw (host.dims (), syncer.frame_time (), syncer.alpha ());

      gl_ctx.flip ();
    }

    return 0; // shut up clang ffs
  }
  catch (const std::exception& e) {
    fprintf (stderr, "Exception!\n%s\n", e.what ());
    return 1;
  }
}


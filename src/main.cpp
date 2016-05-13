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
#include "chunk-source.hpp"

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

  auto make_test_chunkdata () {
    ChunkData chunk;
    for (auto i : chunk.indices ()) {
      int const a = i.z + 1,
                b = 15 - i.z;
      bool const empty = (i.x < a || i.x > b || i.y < a || i.y > b);
      chunk[i] = empty? 0 : 1;
    }
    return std::make_shared<ChunkData> (chunk);
  }

  auto make_blank_chunkdata () {
    ChunkData blank;
    for (auto i : blank.indices ())
      blank[i] = 0;
    return std::make_shared<ChunkData> (blank);
  }

  class TestChunkSource final : public ChunkSource {
    ChunkData::Shared test_chunk = make_test_chunkdata (),
                      blank_chunk = make_blank_chunkdata ();

  public:
    ChunkData::Shared get (vec3i pos) override {
      if (pos.z == -1 && (pos.x % 2 == 0) && (pos.y % 2 == 0))
        return test_chunk;
      else
        return blank_chunk;
    }

    void store (vec3i pos, ChunkData::Shared)
    { }
  };

  auto load_chunk (ChunkSource& source, vec3i pos) -> StageChunk {
    return StageChunk (source.get (pos), pos);
  }

  bool get_adjacent_chunk_datas (
    Array<3, ChunkData const*>& adjs,
    Stage const& stage,
    vec3i const pos)
  {
    static auto const offsets = Array<3, vec3i> {{ v3i{1,0,0}, v3i{0,1,0}, v3i{0,0,1} }};
    for (auto i = 0; i != 3; i++) {
      auto adj_chunk = stage.at_absolute (pos + offsets[i]);
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

  void load_chunks_into_stage (
    ChunkSource& source,
    Stage& stage,
    std::vector<vec3i>& required_chunks
  ) {
    if (required_chunks.empty ())
      return;

    // std::cerr << "generating chunks... ";

    for (vec3i chunk_pos : required_chunks) {
      // std::cerr << chunk_pos << " ";

      // load or generate chunk at chunk_pos
      auto chunk = load_chunk (source, chunk_pos);

      stage.insert (std::move (chunk), chunk_pos);
    }

    // std::cerr << "done\n";
  }

  void generate_meshes (Stage& stage, std::vector<vec3i>& required_chunks) {
    if (required_chunks.empty ())
      return;

    for (vec3i pos : required_chunks) {
      auto chunk = stage.at_absolute (pos);
      // assert (!!chunk && !chunk->mesh);

      Array<3, ChunkData const*> adjs;
      if (!get_adjacent_chunk_datas (adjs, stage, pos))
        continue;

      chunk->regen_mesh (adjs);
    }
  }

  void run () {
    // subsystems
    auto host = XHost::create ();
    auto gl_ctx = GLContext::establish (host.egl_display (), host.egl_window ());
    configure_gl ();

    TestChunkSource source;

    // persistent state
    Syncer syncer;

    Stage stage (9);
    Camera camera (nil);
    camera.dir = v3f{1,0,0};

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
      Camera old_camera = camera;

      syncer.update ();
      while (syncer.need_tick ()) {
        syncer.begin_tick ();
        old_camera = camera;
        camera.pos.x += 1.0f;
      }

      vec3i camera_chunk = floor (camera.pos) / 16;

      // update stage
      stage.relocate (camera_chunk, required_chunks);
      load_chunks_into_stage (source, stage, required_chunks);

      // generate meshes
      if (!required_chunks.empty ()) {
        auto const needs_mesh = [] (StageChunk const& c) { return !c.mesh_ok; };
        stage.find_all_if (needs_mesh, required_chunks);
        generate_meshes (stage, required_chunks);
      }

      // redraw
      for (auto idx : stage.indices ()) {
        auto chunk = stage.at_relative (idx);
        if (!chunk || !chunk->mesh_ok)
          continue;

        frame.add_cmesh (chunk->mesh.get (), chunk->position * vec3i {16,16,16});
      }

      frame.set_camera (old_camera, camera);

      frame.draw (host.dims (), syncer.frame_time (), syncer.alpha ());

      gl_ctx.flip ();
    }
  }
}

int main () try {
  nd::run ();
}
catch (std::exception const& e) {
  std::cerr << "Exception:\n" << e.what () << "\n";
  return 1;
}


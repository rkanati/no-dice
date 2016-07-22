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
#include "test-chunk-source.hpp"
#include "key.hpp"

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
    glEnable (GL_MULTISAMPLE);
    glEnable (GL_CULL_FACE);
    glEnable (GL_DEPTH_TEST);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_COLOR_ARRAY);
  }

  bool get_adjacent_chunk_datas (
    Array<3, ChunkData const*>& adjs,
    Stage const& stage,
    vec3i const pos)
  {
    static v3i const offsets[] = { v3i{1,0,0}, v3i{0,1,0}, v3i{0,0,1} };

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

  void generate_meshes (Stage& stage, std::vector<vec3i> const& required_chunks) {
    for (vec3i pos : required_chunks) {
      auto chunk = stage.at_absolute (pos);

      Array<3, ChunkData const*> adjs;
      if (!get_adjacent_chunk_datas (adjs, stage, pos))
        continue;

      chunk->regen_mesh (adjs);
    }
  }

  void run () {
    // subsystems
    auto host = XHost::create ();
    auto gl_ctx = GLContext::establish (host->egl_display (), host->egl_window ());
    configure_gl ();

    auto source = make_test_chunk_source ();

    // persistent state
    Syncer syncer;

    Stage stage (13);

    Camera camera (v3f {200.0f, -300.0f, 50.f}, identity);
    versf camera_yaw (identity);
    float camera_pitch = 0.0f;

    v2f mouse_prev (nil);
    bool mouse_look = false;

    bool x_posve_go = false,
         x_negve_go = false,
         y_posve_go = false,
         y_negve_go = false;

    // transient state
    std::vector<vec3i> required_chunks;
    Frame frame;

    // main loop
    while (true) {
      // handle events
      auto input = host->pump ();
      if (input.quit)
        break;

      v2i mouse_delta {0,0};

      for (auto const& ev : input.events) {
        if (ev.cause.device == host->keyboard ()) {
          switch ((Key) ev.cause.index) {
            case Key::w: x_posve_go = ev.state; break;
            case Key::s: x_negve_go = ev.state; break;
            case Key::a: y_posve_go = ev.state; break;
            case Key::d: y_negve_go = ev.state; break;
            default:;
          }
        }
        else if (ev.cause.device == host->pointer ()) {
          if (ev.type == InputType::axial_2i) {
            mouse_delta += ev.value_2i - mouse_prev;
            mouse_prev = ev.value_2i;
          }
          else if (ev.type == InputType::bistate) {
            mouse_look = ev.state;
          }
        }
      }

      v3f camera_move{0,0,0};

      if (x_posve_go != x_negve_go) {
        if (x_posve_go) camera_move.x = 1.0f;
        else            camera_move.x = -1.0f;
      }

      if (y_posve_go != y_negve_go) {
        if (y_posve_go) camera_move.y = 1.0f;
        else            camera_move.y = -1.0f;
      }

      camera_move = unit (camera_move);

      if (mouse_look) {
        v2f camera_rotate = mouse_delta * 0.002f;
        camera_yaw = rotation (-camera_rotate.x, v3f{0,0,1}) * camera_yaw;
        camera_pitch += camera_rotate.y;
        camera_pitch = std::min (std::max (camera_pitch, -1.5f), 1.5f);

        camera.ori = camera_yaw * rotation (camera_pitch, v3f{0,1,0});
      }

      camera_move = conj (camera.ori, camera_move);

      // advance simulation
      Camera old_camera = camera;

      syncer.update ();
      while (syncer.need_tick ()) {
        syncer.begin_tick ();
        old_camera = camera;
        camera.pos += camera_move;
      }

      vec3i camera_chunk = floor (camera.pos) / 16;

      // update stage
      stage.relocate (camera_chunk, required_chunks);

      // load or generate chunks
      for (vec3i chunk_pos : required_chunks) {
        auto data = source->get (chunk_pos);
        stage.insert (StageChunk (std::move (data), chunk_pos));
      }

      // generate meshes
      auto const needs_mesh = [] (StageChunk const& c) { return !c.mesh_ok; };
      stage.find_all_if (needs_mesh, required_chunks);
      generate_meshes (stage, required_chunks);

      // redraw
      for (auto idx : stage.indices ()) {
        auto chunk = stage.at_relative (idx);
        if (!chunk || !chunk->mesh_ok)
          continue;

        frame.add_cmesh (chunk->mesh.get (), chunk->position * vec3i {16,16,16});
      }

      frame.set_camera (old_camera, camera);

      frame.draw (host->dims (), syncer.frame_time (), syncer.alpha ());

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


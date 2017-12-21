//
// no-dice
//

#include "test-chunk-source.hpp"
#include "chunk-cache.hpp"
#include "stage-chunk.hpp"
#include "chunk-mesh.hpp"
#include "glcontext.hpp"
#include "work-pool.hpp"
#include "frustum.hpp"
#include "player.hpp"
#include "syncer.hpp"
#include "chunk.hpp"
#include "frame.hpp"
#include "stage.hpp"
#include "types.hpp"
#include "font.hpp"
#include "xwin.hpp"
#include "key.hpp"

#include <cassert>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#include <stdexcept>
//#include <charconv>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <memory>
#include <vector>
#include <array>

#include <epoxy/gl.h>

namespace nd {
  uint make_test_texture (WorkPool&);

namespace {
  void configure_gl () {
    glLineWidth (3.f);
    glEnable (GL_CULL_FACE);
    glEnable (GL_DEPTH_TEST);
  }

  bool mouse_look = false;
  v2i  mouse_prev = nil;
  bool wireframe = false;

  void update_input (InputFrame const& input, XHost& host, Player& player) {
    v2i mouse_delta {0,0};

    for (auto const& ev : input.events) {
      if (ev.cause.device == host.keyboard ()) {
        switch ((Key) ev.cause.index) {
          case Key::w: player.x_move.positive (ev.state); break;
          case Key::a: player.y_move.positive (ev.state); break;
          case Key::s: player.x_move.negative (ev.state); break;
          case Key::d: player.y_move.negative (ev.state); break;

          case Key::l: if (!ev.state) {
            wireframe = !wireframe;
            glPolygonMode (GL_FRONT_AND_BACK, wireframe? GL_LINE : GL_FILL);
          };

          default:;
        }
      }
      else if (ev.cause.device == host.pointer ()) {
        if (ev.type == InputType::axial_2i) {
          if (mouse_look)
            player.look_delta += ev.value_2i - mouse_prev;
          mouse_prev = ev.value_2i;
        }
        else if (ev.type == InputType::bistate) {
          if (ev.cause.index == 1)
            mouse_look = ev.state;
          else if (ev.cause.index == 2)
            player.use = ev.state;
        }
      }
    }
  }

  bool update_stage (
    WorkPool&         pool,
    Stage&            stage,
    v3f               viewpoint,
    ChunkSource&      source,
    ChunkCache&       cache,
    std::vector<v3i>& scratch)
  {
    v3i new_centre = floor (viewpoint) / chunk_dim;

    auto& to_load = stage.relocate (new_centre, scratch);

    std::sort (
      to_load.begin (), to_load.end (),
      [new_centre] (v3i lhs, v3i rhs) {
        return abs2 (lhs-new_centre) < abs2 (rhs-new_centre);
      }
    );

    // load or generate chunks
    for (v3i chunk_pos : to_load) {
      if (auto data = cache.load (chunk_pos)) {
        stage.update_data (std::move (data), chunk_pos);
      }
      else {
        pool.nq ([chunk_pos, &source, &cache, &stage] (WorkPool& pool) {
          auto data = source.get (chunk_pos);
          pool.complete ([chunk_pos, d=std::move(data), &cache, &stage] {
            cache.store (chunk_pos, d);
            stage.update_data (std::move (d), chunk_pos);
          });
        });
      }
    }

    return true;
  }

  bool get_adjacent_chunk_datas (
    std::array<ChunkData const*, 4>& adjs,
    Stage const& stage,
    v3i const pos)
  {
    std::array <v3i, 4> const constexpr offsets {
      v3i{0,0,0}, v3i{1,0,0}, v3i{0,1,0}, v3i{0,0,1}
    };

    for (auto i = 0; i != offsets.size (); i++) {
      auto want_pos = pos + offsets[i];
      auto chunk = stage.at_absolute (want_pos);
      auto const* data = chunk->get_data (want_pos).get ();
      if (!data)
        return false;
      adjs[i] = data;
    }

    return true;
  }

  void sync_meshes (
    ChunkMesher& mesher,
    Stage& stage,
    std::vector<v3i>& scratch,
    int limit = 5)
  {
    auto to_mesh = stage.remesh (scratch);

    for (v3i chunk_pos : to_mesh) {
      std::array<ChunkData const*, 4> adjs;
      if (!get_adjacent_chunk_datas (adjs, stage, chunk_pos) || limit == 0) {
        stage.cancel_meshing (chunk_pos);
        continue;
      }

      auto mesh = mesher.build (adjs);
      stage.at_absolute (chunk_pos)->update_mesh (std::move (mesh), chunk_pos);
      limit--;
    }
  }

  void draw_stage (Stage const& stage, Frustum const& frustum, Frame& frame) {
    for (auto rel : stage.indices ()) {
      if (!stage.in_radius (rel))
        continue;

      auto pos = stage.abs_for_rel (rel);
      auto mesh = stage.at_relative (rel)->get_mesh (pos);
      if (!mesh)
        continue;

      if (!frustum.intersects_cube (chunk_dim*pos, chunk_dim))
        continue;

      frame.add_chunk (std::move (mesh), chunk_dim*pos, abs2 (rel));
    }
  }

  // parameters
#ifndef NDEBUG
  auto const stage_radius = 256;
#else
  auto const stage_radius = 512;
#endif

  float const fov = 75.f;

  void run () {
    // subsystems
    auto host = XHost::create ();
    auto gl_ctx = GLContext::establish (host->egl_window ());
    configure_gl ();

    auto source = make_test_chunk_source ();
    auto mesher = make_chunk_mesher ();
    auto font_loader = make_font_loader ();

    WorkPool pool (4);

    // persistent state
    Syncer syncer;
    ChunkCache cache;
    Stage stage (stage_radius);
    Player player (v3f {0.f, 0.f, 17.f});
    v2i mouse_prev;

    // transient state
    std::vector<v3i> scratch;
    Frame frame;
    double avg_frame_time = 0.f;

    // assets
    uint tex = make_test_texture (pool);

    /*auto font = font_loader->load_scalable (
      "/usr/share/fonts/OTF/OfficeCodeProD-Regular.otf", 20);*/
    auto font = font_loader->load_scalable (
      "/usr/share/fonts/TTF/LiberationMono-Regular.ttf");
    auto const latin1 = CharRanges { CharRange {0x00, 0xff} };
    font->prime (latin1);
    font->fix ();

    // main loop
    for (;;) {
      pool.run_completions ();

      // handle events
      auto input = host->pump ();
      if (input.quit)
        break;

      update_input (input, *host, player);

      // advance simulation
      double frame_time = syncer.update ();
      while (syncer.need_tick ()) {
        syncer.begin_tick ();
        player.advance (syncer.time_step);
      }

      // update stage
      update_stage (pool, stage, player.position (), *source, cache, scratch);

      // generate meshes
      sync_meshes (*mesher, stage, scratch);

      // draw stage
      auto camera = lerp (
        player.prev_placement (), player.placement (), syncer.alpha ());
      Frustum frustum (camera, fov, host->aspect ());
      draw_stage (stage, frustum, frame);

      // update fps
      { double const constexpr n = 40.;
        avg_frame_time = ((n - 1.)/n)*avg_frame_time + (1./n)*frame_time;
        char fps_buf[6];
        snprintf (fps_buf, 6, "%5.1f", 1./avg_frame_time);
        auto fps_msg = font->bake (fps_buf);
        draw_text (frame, fps_msg, v2i{10,10}, 0.5f);
      }

      frame.set_frustum (frustum);

      glBindTexture (GL_TEXTURE_2D_ARRAY, tex);
      frame.draw (host->dims (), syncer.frame_time (), syncer.alpha ());
      gl_ctx.flip ();
    }

    font->dump ("font-dump.ppm");
  }
}
} // nd

namespace {
  void show_exception (std::exception const& e) {
    std::cerr << "Exception:\n" << e.what () << "\n";

    try {
      std::rethrow_if_nested (e);
    }
    catch (std::exception const& e) {
      show_exception (e);
    }
  }
}

int main () try {
  nd::run ();
}
catch (std::exception const& e) {
  show_exception (e);
  return 1;
}



#include "frame.hpp"

#include <stdexcept>
#include <algorithm>

#include <Rk/file_stream.hpp>

#include <epoxy/gl.h>

namespace nd {
  Frame::Frame () :
    chunk_renderer (make_chunk_renderer ())
  {
    glCreateVertexArrays (1, &ui_vao);

    glEnableVertexArrayAttrib  (ui_vao, 1);
    glVertexArrayAttribFormat  (ui_vao, 1, 2, GL_INT, false, 0);
    glVertexArrayAttribBinding (ui_vao, 1, 1);

    glEnableVertexArrayAttrib  (ui_vao, 2);
    glVertexArrayAttribFormat  (ui_vao, 2, 2, GL_INT, false, 8);
    glVertexArrayAttribBinding (ui_vao, 2, 1);

    glCreateBuffers (1, &ui_buf);
    glVertexArrayVertexBuffer (ui_vao, 1, ui_buf, 0, 16);

    ui_prog = link (
      load_shader ("assets/shaders/ui.vert", ShaderType::vertex),
      load_shader ("assets/shaders/ui.frag", ShaderType::fragment)
    );
  }

  void Frame::draw (v2i dims, float, float /*alpha*/) {
    dims.x = std::max (dims.x, 1);
    dims.y = std::max (dims.y, 1);

    glViewport (0, 0, dims.x, dims.y);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /*v3f sky_top    {0.05f, 0.15f, 0.60f},
        sky_bottom {0.15f, 0.30f, 0.95f};*/

    // render world chunks
    std::sort (chunk_items.begin (), chunk_items.end (),
      [] (auto const& a, auto const& b) { return a.z < b.z; });
    auto w2c = e2c * w2e;
    glEnable (GL_DEPTH_TEST);
    glDisable (GL_BLEND);
  //glDepthMask (true);
    chunk_renderer->draw (w2c, chunk_items);
    chunk_items.clear ();

    // render ui
    ui_prog.use ();
    glBindVertexArray (ui_vao);
    glNamedBufferData (
      ui_buf,
      sizeof (UIRect) * ui_rects.size (), ui_rects.data (),
      GL_DYNAMIC_DRAW
    );

    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glDepthMask (false);
    std::sort (ui_items.begin (), ui_items.end ());
    glUniform2f (1, 2.f/dims.x, 2.f/dims.y);
    uint prev_texture = 0;
    for (auto const& item : ui_items) {
      if (item.texture != prev_texture)
        glBindTexture (GL_TEXTURE_2D, item.texture);
      glUniform2f (2, item.pos.x, item.pos.y);
      prev_texture = item.texture;
      glDrawArrays (GL_TRIANGLES, item.begin*6, item.count*6);
    }
    ui_items.clear ();
    ui_rects.clear ();
  }
}


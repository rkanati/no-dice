
#include "frame.hpp"

#include <stdexcept>
#include <algorithm>

#include <Rk/file_stream.hpp>

#include <epoxy/gl.h>

namespace nd {
  Frame::Frame () :
    chunk_renderer (make_chunk_renderer ())
  { }

  void Frame::draw (v2i dims, float, float /*alpha*/) {
    dims.x = std::max (dims.x, 1);
    dims.y = std::max (dims.y, 1);

    glViewport (0, 0, dims.x, dims.y);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /*v3f sky_top    {0.05f, 0.15f, 0.60f},
        sky_bottom {0.15f, 0.30f, 0.95f};*/

    std::sort (chunk_items.begin (), chunk_items.end (),
      [] (auto const& a, auto const& b) { return a.z < b.z; });
    auto w2c = e2c * w2e;
    chunk_renderer->draw (w2c, chunk_items);
    chunk_items.clear ();
  }
}


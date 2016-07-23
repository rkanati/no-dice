//
// no-dice
//

#include "perlin.hpp"

#include "vec-iter.hpp"

#include <Rk/lerp.hpp>

#include <GL/gl.h>

namespace nd {
  uint make_test_texture () {
    using Rk::lerp;

    int const dim = 256;
    auto const scale = 4.0f / dim;

    v3f data [dim * dim];
    CoordHasher hasher (346565);

    for (auto i : vec_range (v2i{0,0}, v2i{dim,dim})) {
      float value = perlin (hasher, i * scale, 4);
      v3f colour = lerp (v3f{0.3f,0.1f,0.0f}, v3f{0.1f,0.7f,0.1f}, value);
      data [i.y * dim + i.x] = colour;
    }

    uint tex;
    glGenTextures (1, &tex);
    glBindTexture (GL_TEXTURE_2D, tex);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, dim, dim, 0, GL_RGB, GL_FLOAT, data);
    return tex;
  }
}


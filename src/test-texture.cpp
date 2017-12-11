//
// no-dice
//

#include "vec-iter.hpp"
#include "perlin.hpp"
#include "smooth.hpp"

#include <Rk/lerp.hpp>

#include <epoxy/gl.h>

namespace nd {
  uint make_test_texture () {
    using Rk::lerp;

    #ifndef NDEBUG
    int const dim = 8;
    #else
    int const dim = 32;
    #endif

    std::array<v3f, 3> const primaries {
      v3f{1.0f,0.0f,0.0f},
      v3f{0.0f,1.0f,0.0f},
      v3f{0.0f,0.0f,1.0f}
    };

    uint tex;
    glGenTextures (1, &tex);
    glBindTexture (GL_TEXTURE_2D_ARRAY, tex);
    glTexStorage3D (GL_TEXTURE_2D_ARRAY, 2, GL_RGB8, dim, dim, 256);

    for (int i = 0; i != 256; i++) {
      int const
        n_slabs = primaries.size (),
        slab_depth = 256 / n_slabs,
        slab = i/slab_depth;
      float slab_rel = (1.f/slab_depth) * (i - slab_depth*slab);
      v3f const
        colour = unit (lerp (
          primaries[ slab    % n_slabs],
          primaries[(slab+1) % n_slabs],
          slab_rel
        )),
        inv_colour = lerp (v3f{0,0,0}, v3f{1,1,1}, smoothstep (i*(1.f/256)));

      v3f data [dim * dim];
      int const s = 947441244;
      VecHash h[3] { VecHash (s+i), VecHash (s+256+i), VecHash (s+512+i) };

      for (auto pos : vec_range (v2i{0,0}, v2i{dim,dim})) {
        int const freq = 32;
        v2f const noise_pos = pos * (float (freq) / dim);
        v2f const off{0.37f,0.69f};

        float const
          k = 5.f/21 * perlin (h[0],   noise_pos+off,   freq)
            + 7.f/21 * perlin (h[1], 2*noise_pos+off, 2*freq)
            + 9.f/21 * perlin (h[2], 4*noise_pos+off, 4*freq);

        data [pos.y*dim + pos.x] = (0.2f + 0.8f*k*k)*colour;
      }

      glTexSubImage3D (
        GL_TEXTURE_2D_ARRAY,
        0,
        0, 0, i,
        dim, dim, 1,
        GL_RGB,
        GL_FLOAT, data
      );
    }

    v2i constexpr const attribs[] {
      { GL_TEXTURE_MIN_FILTER,          GL_LINEAR_MIPMAP_LINEAR },
      { GL_TEXTURE_MAG_FILTER,          GL_NEAREST },
      { GL_TEXTURE_WRAP_S,              GL_REPEAT },
      { GL_TEXTURE_WRAP_T,              GL_REPEAT },
      { GL_TEXTURE_MAX_ANISOTROPY_EXT,  1 }
    };
    for (v2i attr : attribs)
      glTexParameteri (GL_TEXTURE_2D_ARRAY, attr.x, attr.y);

    glGenerateMipmap (GL_TEXTURE_2D_ARRAY);

    return tex;
  }
}


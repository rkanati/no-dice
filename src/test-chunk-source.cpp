//
// no-dice
//

#include "test-chunk-source.hpp"

#include "perlin.hpp"
#include "grid-filter.hpp"
#include "smooth.hpp"

#include <Rk/clamp.hpp>

#include <cmath>

namespace nd {
  class TestChunkSource final : public ChunkSource {
    VecHash hasher;
    ChunkData::Shared empty_chunk, full_chunk;

    static auto make_empty_chunkdata () {
      ChunkData chunk;
      for (auto i : chunk.indices ())
        chunk[i] = 0;
      return share (std::move (chunk));
    }

    static auto make_full_chunkdata () {
      ChunkData chunk;
      for (auto i : chunk.indices ())
        chunk[i] = 255;
      return share (std::move (chunk));
    }

  public:
    TestChunkSource () :
      hasher (9009),
      empty_chunk (make_empty_chunkdata ()),
      full_chunk (make_full_chunkdata ())
    { }

    ChunkData::Shared get (vec3i pos) override {
      int constexpr const
        bedrock = -(256/chunk_dim),
        peaks   =  0;

      if (pos.z < bedrock) {
        return full_chunk;
      }
      else if (pos.z >= peaks) {
        return empty_chunk;
      }
      else {
        ChunkData chunk;

        // sample nested function
        auto noise = [this] (v2f x) {
          return
              5.f/6 * perlin (hasher,      x                 )
            + 1.f/6 * perlin (hasher, 11.3*x + v2f{0.5f,0.5f});
        };

        float const freq = 0.5f;
        using namespace Rk::swiz;
        auto samples = take_grid_samples<2> (pos(X,Y)*freq, freq, noise);

        for (auto i : chunk.indices ()) {
          float const
            above = ((pos.z - bedrock) * chunk_dim + i.z),
            scale = 1.f / (float (chunk_dim) * (peaks - bedrock)),
            alt = above * scale,
            density = smooth_filter (samples, 1.f/chunk_dim * i(X,Y)),
            s_alt = smoothstep (alt);
          chunk[i] = alt < density? u8 (int (s_alt*254.f) + 1) : 0;
        }

        return share (std::move (chunk));
      }
    }

    void store (vec3i, ChunkData::Shared)
    { }
  };

  SharePtr<ChunkSource> make_test_chunk_source () {
    return std::make_shared<TestChunkSource> ();
  }
}


//
// no-dice
//

#include "test-chunk-source.hpp"

#include "perlin.hpp"
#include "grid-filter.hpp"

#include <cmath>

namespace nd {
  class TestChunkSource final : public ChunkSource {
    CoordHasher hasher;

    static auto make_empty_chunkdata () {
      ChunkData chunk;
      for (auto i : chunk.indices ())
        chunk[i] = 0;
      return share (std::move (chunk));
    }

    static auto make_full_chunkdata () {
      ChunkData chunk;
      for (auto i : chunk.indices ())
        chunk[i] = 1;
      return share (std::move (chunk));
    }

  public:
    TestChunkSource () :
      hasher (9009)
    { }

    ChunkData::Shared get (vec3i pos) override {
      static SharePtr<ChunkData> empty_chunk = make_empty_chunkdata (),
                                 full_chunk  = make_full_chunkdata ();

      static int const bedrock = -10,
                       peaks   =   0;

      if (pos.z < bedrock) {
        return full_chunk;
      }
      else if (pos.z >= peaks) {
        return empty_chunk;
      }
      else {
        ChunkData chunk;

        // sample nested function
        auto noise = [this] (v3f x) {
          return perlin (hasher, x);
        };

        float const freq = 0.25f;
        auto samples = take_grid_samples<2> (pos * freq, freq, noise);

        for (auto i : chunk.indices ()) {
          float noise_value = linear_filter (samples, i * (1.0f / 16.0f));
          auto alt = ((pos.z - bedrock) * 16 + i.z) / (16.0f * (peaks - bedrock));
          float alt_value = 1.0f - alt;
          float interp = 2.0f * (alt - 0.5f);
          interp *= interp;
          float density = Rk::lerp (noise_value, alt_value, interp);
          chunk[i] = density > 0.5f? 1 : 0;
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


//
// no-dice
//

#include "chunk-cache.hpp"

#include <unordered_map>

namespace nd {
  struct VecHash {
    std::size_t operator () (vec3i v) const {
      size_t hash = 0;
      for (int i = 0; i != 3; i++)
        hash ^= u32 (v[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      return hash;
    }
  };

  struct ChunkCache::Impl {
    using Entry = std::shared_ptr <ChunkData>;

    std::unordered_map <vec3i, Entry, VecHash> map;
    ChunkSource& source;

    Impl (ChunkSource& src) :
      source (src)
    { }
  };

  ChunkCache::ChunkCache (ChunkSource& src) :
    impl (new Impl (src))
  { }

  ChunkCache::~ChunkCache () {
    delete impl;
  }

  auto ChunkCache::load (vec3i chunk_pos) -> std::shared_ptr <ChunkData> {
    std::shared_ptr <ChunkData> chunk_ptr;

    auto iter = impl->map.find (chunk_pos);
    if (iter != impl->map.end ()) {
      chunk_ptr = iter->second;
    }
    else {
      chunk_ptr = impl->source.load (chunk_pos);
      impl->map.insert (iter, std::make_pair (chunk_pos, chunk_ptr));
    }

    return chunk_ptr;
  }

  void ChunkCache::store (vec3i chunk_pos, const ChunkData& chunk) {
    impl->source.store (chunk_pos, chunk);
  }
}


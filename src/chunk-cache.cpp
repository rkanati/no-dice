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
    using Entry = std::shared_ptr<ChunkData>;
    std::unordered_map<vec3i, Entry, VecHash> map;
  };

  ChunkCache::ChunkCache () :
    impl (new Impl ())
  { }

  ChunkCache::~ChunkCache () {
    delete impl;
  }

  auto ChunkCache::load (vec3i chunk_pos) -> std::shared_ptr<ChunkData> {
    auto iter = impl->map.find (chunk_pos);
    if (iter == impl->map.end ())
      return nullptr;

    return iter->second;
  }

  void ChunkCache::store (vec3i chunk_pos, std::shared_ptr<ChunkData> chunk) {
    impl->map.insert (impl->map.end (), std::make_pair (chunk_pos, std::move (chunk)));
  }
}


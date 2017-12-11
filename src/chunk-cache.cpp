//
// no-dice
//

#include "chunk-cache.hpp"

#include "vector.hpp"

#include <unordered_map>

namespace nd {
  struct ChunkCache::Impl {
    std::unordered_map<v3i, ChunkData::Shared, VecHash> map;
  };

  ChunkCache::ChunkCache () :
    impl (new Impl ())
  { }

  ChunkCache::~ChunkCache () {
    delete impl;
  }

  ChunkData::Shared ChunkCache::load (v3i chunk_pos) const {
    auto iter = impl->map.find (chunk_pos);
    if (iter == impl->map.end ())
      return nullptr;

    return iter->second;
  }

  void ChunkCache::store (v3i chunk_pos, ChunkData::Shared chunk) {
    impl->map.insert (
      impl->map.end (),
      std::make_pair (chunk_pos, std::move (chunk))
    );
  }
}


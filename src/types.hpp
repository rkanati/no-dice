//
// no-dice
//

#pragma once

#include <Rk/types.hpp>
#include <Rk/string_ref.hpp>

#include <array>
#include <memory>
#include <utility>

namespace nd {
  using StrRef = Rk::StringRef;

  template<typename T>
  using Shared = std::shared_ptr<T>;
}


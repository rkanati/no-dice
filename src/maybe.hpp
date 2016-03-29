
#pragma once

#include <experimental/optional>

namespace nd {
  template<typename T>
  using Maybe = std::experimental::optional<T>;

  template<typename T>
  auto just (T&& value) {
    return Maybe<T> (std::forward<T> (value));
  }

  auto constexpr static nothing = std::experimental::nullopt;
}


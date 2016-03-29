//
// no-dice
//

#pragma once

#include <Rk/types.hpp>

#include <array>
#include <memory>
#include <utility>

namespace nd {
  template<size_t n, typename T>
  using Array = std::array<T, n>;

  template<typename T>
  using SharePtr = std::shared_ptr<T>;

  template<typename T>
  static inline auto share (T&& value) {
    return std::make_shared<std::decay_t <T>> (std::forward<T> (value));
  }

  namespace {
    template<typename Fn, typename T, size_t... idxs>
    static auto map_impl (Fn fn, std::initializer_list<T> const& in, std::integer_sequence <size_t, idxs...>)
      -> std::initializer_list<decltype (fn (std::declval<T> ()))>
    {
      return { fn (in[idxs]) ... };
    }
  }

  template<typename Fn, typename T>
  static auto map (Fn fn, std::initializer_list<T> const& in) {
    return map_impl (fn, in, std::make_index_sequence<in.size ()> ());
  }
}


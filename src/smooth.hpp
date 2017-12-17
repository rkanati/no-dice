//
// no-dice
//
#pragma once

namespace nd {
namespace {
  template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
  inline T smoothstep (T t) {
    return (-2*t + 3)*t*t;
  }

  template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
  inline T smoothstep2 (T t) {
    return ((6*t - 15)*t + 10)*t*t*t;
  }
}
}


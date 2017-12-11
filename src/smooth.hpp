//
// no-dice
//
#pragma once

namespace nd {
  static inline float smoothstep (float x) {
    return x*x*(3.f - 2.f*x);
  }
}


//
// no-dice
//

#pragma once

#include "types.hpp"
#include "vector.hpp"

#include <vector>

namespace nd {
  enum class InputType {
    bistate,
    axial_1i,
    axial_1f,
    axial_2i,
    axial_2f
  };

  class InputDevice {
  };

  struct InputCause {
    InputDevice* device;
    uint    index;
  };

  struct InputEvent {
    InputCause cause;
    InputType  type;
    bool       relative;
    union {
      bool     state;
      int      value_1i;
      v2i      value_2i;
      float    value_1f;
      v2f      value_2f;
    };
  };

  using InputEventQueue = std::vector<InputEvent>;

  struct InputFrame {
    bool quit;
    InputEventQueue events;

    InputFrame () :
      quit (false)
    { }
  };
}


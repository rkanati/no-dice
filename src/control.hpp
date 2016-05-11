
#pragma once

#include "input.hpp"

namespace nd {
  class Control {
  };

  struct ControlBinding {
    InputCause cause;
    Control*   control;
  };

  class ControlMap {
  public:
    void apply (std::vector<InputEvent>&);
  };
}


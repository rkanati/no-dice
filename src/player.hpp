//
// no-dice
//

#include "vector.hpp"
#include "versor.hpp"
#include "frame.hpp"

namespace nd {
  struct ButtonPairAxisControl {
    bool positive = false,
         negative = false;

    float value () const {
      if (positive != negative) {
        if (positive) return 1.0f;
        else return -1.0f;
      }
      else {
        return 0.0f;
      }
    }
  };

  class Player {
    v3f   pos;
    versf ori = identity;
    versf yaw = identity;
    float pitch = 0.0f;
    Placement prev_pment;

  public:
    ButtonPairAxisControl x_move, y_move;
    v2i look_delta = nil;
    bool use = false;

    explicit Player (v3f pos) :
      pos (pos)
    { }

    v3f position () const {
      return pos;
    }

    versf orientation () const {
      return ori;
    }

    Placement placement () const {
      return { pos, ori };
    }

    Placement prev_placement () const {
      return prev_pment;
    }

    void advance (float dt);
  };
}


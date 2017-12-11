//
// no-dice
//

#include "vector.hpp"
#include "versor.hpp"
#include "frame.hpp"

namespace nd {
  class ButtonPairAxisControl {
    enum State { none, one, both } state = none;
    enum Dir { pos, neg } dir = pos;

    void change (bool down, Dir intent, Dir opposite) {
      if (down) {
        if (state == none) {
          dir = intent;
          state = one;
        }
        else if (state == one && dir != intent) {
          dir = intent;
          state = both;
        }
      }
      else {
        if (state == one && dir == intent) {
          state = none;
        }
        else if (state == both) {
          dir = opposite;
          state = one;
        }
      }
    }

  public:
    void positive (bool down) {
      change (down, pos, neg);
    }

    void negative (bool down) {
      change (down, neg, pos);
    }

    float value () const {
      if (state == none)
        return 0.f;

      switch (dir) {
        case pos: return  1.f;
        default:  return -1.f;
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


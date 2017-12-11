//
// no-dice
//

#include "player.hpp"

#include <Rk/clamp.hpp>

#include <algorithm>

namespace nd {
  void Player::advance (float dt) {
    prev_pment = placement ();

    float const speed = 50.0f;
    v3f move = dt * speed * unit (v3f { x_move.value (), y_move.value (), 0.0f });

    // float look_len = length (look_delta);
    v2f look_step = look_delta;
    // if (look_len > 0.1f) {
    //   auto look_scale = std::min (20.0f, look_len);
    //   look_step = (look_delta / look_len) * look_scale;
    //   look_delta -= look_step;
    // }
    look_delta = nil;

    v2f rotate = look_step * 0.0015;
    yaw = rotation (-rotate.x, v3f{0,0,1}) * yaw;
    pitch = Rk::clamp (pitch + rotate.y, -1.5f, 1.5f);
    ori = yaw * rotation (pitch, v3f{0,1,0});

    move = conj (ori, move);
    pos += move;
  }
}



#pragma once

#include <time.h>

namespace nd {
  class Syncer {
    double prev_real_time;
    double game_time;
    double accumulator;

    static double now () {
      timespec ts { };
      clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
      return double (ts.tv_sec) + ts.tv_nsec * 0.000000001;
    }

  public:
    const double tick_hz = 50.0;
    const double time_step = 1.0 / tick_hz;

    Syncer () :
      prev_real_time (now ()),
      game_time      (0.0),
      accumulator    (0.0)
    { }

    void update () {
      double real_time = now ();
      accumulator += real_time - prev_real_time;
      prev_real_time = real_time;
    }

    bool need_tick () const {
      return accumulator >= time_step;
    }

    void begin_tick () {
      accumulator -= time_step;
      game_time   += time_step;
    }

    double time () const {
      return game_time;
    }

    double frame_time () const {
      return game_time + alpha () * time_step;
    }

    float alpha () const {
      return accumulator * tick_hz;
    }
  };
}


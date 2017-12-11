//
// no-dice
//

#include "work-pool.hpp"

#include <iostream>

namespace nd {
  WorkPool::WorkPool (int n) {
    while (n--) {
      std::thread t ([this] { worker (); });
      pool.push_back (std::move (t));
    }
  }

  WorkPool::WorkPool (
    std::function<std::shared_ptr<void>()> make_ctx,
    int n)
  {
    while (n--) {
      std::thread t ([this, make_ctx] {
        auto ctx = make_ctx ();
        worker ();
      });
      pool.push_back (std::move (t));
    }
  }

  WorkPool::~WorkPool () {
    { std::lock_guard<std::mutex> lock (mutex);
      queue.clear ();
      for (auto& t : pool)
        queue.push_back (nullptr);
      cv.notify_all ();
    }

    for (auto& t : pool)
      t.join ();
  }

  void WorkPool::worker () {
    for (;;) {
      Job job;
      { std::unique_lock<std::mutex> lock (mutex);
        while (queue.empty ())
          cv.wait (lock);
        job = std::move (queue.front ());
        queue.pop_front ();
      }

      if (!job)
        break;

      job (*this);
    }
  }

  void WorkPool::run_completions () {
    std::deque<Completion> comps;
    { std::lock_guard<std::mutex> lock (completions_mutex);
      comps = std::move (completions);
    }

    for (auto const& f : comps)
      f ();
  }
}


//
// no-dice
//

#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <deque>
#include <mutex>

namespace nd {
  class WorkPool {
  public:
    using Job        = std::function<void (WorkPool&)>;
    using Completion = std::function<void ()>;

  private:
    std::deque<Job>          queue;
    std::mutex               mutex;
    std::condition_variable  cv;
    std::vector<std::thread> pool;

    std::deque<Completion> completions;
    std::mutex             completions_mutex;

    void worker ();

  public:
    explicit WorkPool (int n = std::thread::hardware_concurrency ());
    ~WorkPool ();

    void nq (Job job) {
      std::lock_guard<std::mutex> lock (mutex);
      queue.push_back (std::move (job));
      cv.notify_one ();
    }

    void complete (Completion comp) {
      std::lock_guard<std::mutex> lock (completions_mutex);
      completions.push_back (std::move (comp));
    }

    void run_completions ();
  };
}


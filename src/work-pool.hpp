//
// no-dice
//

#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <thread>
#include <deque>
#include <mutex>

namespace nd {
  class WorkPool {
    template<typename Ret, typename... Args>
    class Fun {
      struct Handle {
        virtual Ret invoke (Args...) = 0;
        virtual ~Handle () = default;
      };

      template<typename Clos>
      struct Impl final : Handle {
        Clos clos;
        Ret invoke (Args... args) override { return clos (args...); }
        Impl (Clos clos) : clos (std::move (clos)) { }
        ~Impl () override = default;
        Impl (Impl const&) = delete;
      };

      std::unique_ptr<Handle> handle;

    public:
      Fun () = default;
      Fun (nullptr_t) : handle (nullptr) { }

      template<typename Clos>
      Fun (Clos clos) :
        handle (std::make_unique<Impl<Clos>> (std::move (clos)))
      { }

      Ret operator () (Args... args) const {
        if (!handle)
          throw std::bad_function_call ();
        return handle->invoke (args...);
      }

      Fun (Fun const&) = delete;
      Fun (Fun&&) = default;

      Fun& operator = (Fun const&) = delete;
      Fun& operator = (Fun&&) = default;

      explicit operator bool () const {
        return (bool) handle;
      }
    };

    using Job        = Fun<void, WorkPool&>;
    using Completion = Fun<void>;

    std::deque<Job>          queue;
    std::mutex               mutex;
    std::condition_variable  cv;
    std::vector<std::thread> pool;

    std::deque<Completion> completions;
    std::mutex             completions_mutex;

    void worker ();

    template<typename Fn>
    static Job dispatcher (std::promise<void> prom, Fn fn) {
      return [prom=std::move (prom), f=std::move (fn)] (WorkPool& p) mutable {
        try {
          f (p);
          prom.set_value ();
        }
        catch (...) {
          prom.set_exception (std::current_exception ());
        }
      };
    }

    template<typename R, typename Fn>
    static Job dispatcher (std::promise<R> prom, Fn fn) {
      return [prom=std::move (prom), f=std::move (fn)] (WorkPool& p) mutable {
        try {
          prom.set_value (f (p));
        }
        catch (...) {
          prom.set_exception (std::current_exception ());
        }
      };
    }

  public:
    explicit WorkPool (int n = std::thread::hardware_concurrency ());
    WorkPool (
      std::function<std::shared_ptr<void>()> make_ctx,
      int n = std::thread::hardware_concurrency ()
    );
    ~WorkPool ();

    template<typename Fn, typename R = std::invoke_result_t<Fn, WorkPool&>>
    auto nq (Fn fn) -> std::future<R> {
      std::promise<R> prom;
      auto fut = prom.get_future ();
      auto job = dispatcher (std::move (prom), std::move (fn));

      { std::lock_guard<std::mutex> lock (mutex);
        queue.push_back (std::move (job));
        cv.notify_one ();
      }

      return fut;
    }

    template<typename Comp>
    void complete (Comp comp) {
      std::lock_guard<std::mutex> lock (completions_mutex);
      completions.push_back (std::move (comp));
    }

    void run_completions ();
  };
}


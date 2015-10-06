//
// no-dice
//

#pragma once

namespace nd {
  template <typename value_t, typename func_t>
  class guard_t {
    value_t& ref;
    func_t func;
    bool active;
  public:
    guard_t (value_t& ref, func_t func) :
      ref (ref),
      func (func),
      active (true)
    { }
    guard_t (const guard_t&) = delete;
    guard_t (guard_t&& other) :
      ref (other.ref),
      func (other.func),
      active (other.active)
    {
      other.active = false;
    }
    void relieve () {
      active = false;
    }
    ~guard_t () {
      if (active)
        func (ref);
    }
  };

  template <typename value_t, typename func_t>
  auto guard (value_t& ref, func_t func) {
    return guard_t <value_t, func_t> (ref, func);
  }
}


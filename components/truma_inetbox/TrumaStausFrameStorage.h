#pragma once

#include <atomic>

#include "esphome/core/automation.h"

namespace esphome {
namespace truma_inetbox {

// Storage for a Truma status frame (e.g. heater, aircon, clock).
//
// Thread safety: `set_status` is called from the LIN eventTask_, while
// `get_status*`, `update` and `reset` are called from the main loop.
// The boolean flags use std::atomic to avoid torn reads. The struct `data_`
// itself is not mutex-protected: a write of T from eventTask_ may interleave
// with a read of T from the main loop. Risk is bounded — frames arrive every
// ~100 ms while sensor publishes happen on main loop tick, so collisions are
// rare and self-healing on the next frame.
template<typename T> class TrumaStausFrameStorage {
 public:
  bool get_status_valid() { return this->data_valid_.load(); };
  const T *get_status() { return &this->data_; };
  virtual void set_status(T val) {
    this->data_ = val;
    this->data_valid_.store(true);
    this->data_updated_.store(true);
    this->dump_data();
  };
  void update() {
    if (this->data_updated_.exchange(false)) {
      this->state_callback_.call(&this->data_);
    }
  };
  virtual void reset() {
    this->data_valid_.store(false);
    this->data_updated_.store(false);
  };
  void add_on_message_callback(std::function<void(const T *)> callback) {
    this->state_callback_.add(std::move(callback));
  };
  virtual void dump_data() const = 0;

 protected:
  CallbackManager<void(const T *)> state_callback_{};
  T data_;
  std::atomic<bool> data_valid_{false};
  // Value has changed notify listeners.
  std::atomic<bool> data_updated_{false};
};

}  // namespace truma_inetbox
}  // namespace esphome
#pragma once

#include <atomic>

#include "TrumaStausFrameStorage.h"
#include "TrumaStructs.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxApp;

// Cross-task flags: `update_submit` is called from the main loop (user actions),
// `update_submitted` and the set_status override run on the LIN eventTask_.
// std::atomic<bool> guarantees safe transitions; the `update_status_` struct
// itself is briefly accessed from both tasks (see comment in TrumaStausFrameStorage).
template<typename T, typename TResponse>
class TrumaStausFrameResponseStorage : public TrumaStausFrameStorage<T>, public Parented<TrumaiNetBoxApp> {
 public:
  void reset() override {
    TrumaStausFrameStorage<T>::reset();
    this->update_status_prepared_.store(false);
    this->update_status_unsubmitted_.store(false);
    this->update_status_stale_.store(false);
  }
  virtual bool can_update() { return this->data_valid_.load(); }
  virtual TResponse *update_prepare() = 0;
  void update_submit() { this->update_status_unsubmitted_.store(true); }
  bool has_update() const { return this->update_status_unsubmitted_.load(); }
  void set_status(T val) override {
    TrumaStausFrameStorage<T>::set_status(val);
    this->update_status_stale_.store(false);
  };
  virtual void create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) = 0;

 protected:
  inline void update_submitted() {
    this->update_status_prepared_.store(false);
    this->update_status_unsubmitted_.store(false);
    this->update_status_stale_.store(true);
  }

  // Prepared means `update_status_` was copied from `data_`.
  std::atomic<bool> update_status_prepared_{false};
  // Prepared means an update is already awaiting fetch from CP plus.
  std::atomic<bool> update_status_unsubmitted_{false};
  // I have submitted my update request to CP plus, but I have not received an update with new heater values from CP
  // plus.
  std::atomic<bool> update_status_stale_{false};
  TResponse update_status_;
};

}  // namespace truma_inetbox
}  // namespace esphome
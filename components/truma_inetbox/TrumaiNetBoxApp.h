#pragma once

#include <atomic>

#include "LinBusProtocol.h"
#include "TrumaStructs.h"
#include "TrumaiNetBoxAppAirconAuto.h"
#include "TrumaiNetBoxAppAirconManual.h"
#include "TrumaiNetBoxAppClock.h"
#include "TrumaiNetBoxAppConfig.h"
#include "TrumaiNetBoxAppHeater.h"
#include "TrumaiNetBoxAppTimer.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif  // USE_TIME

namespace esphome {
namespace truma_inetbox {

#define LIN_PID_TRUMA_INET_BOX 0x18

class TrumaiNetBoxApp : public LinBusProtocol {
 public:
  TrumaiNetBoxApp();
  void update() override;

  const std::array<uint8_t, 4> lin_identifier() override;
  void lin_heartbeat() override;
  void lin_reset_device() override;

  TRUMA_DEVICE get_heater_device() const { return this->heater_device_.load(std::memory_order_relaxed); }
  TRUMA_DEVICE get_aircon_device() const { return this->aircon_device_.load(std::memory_order_relaxed); }

  TrumaiNetBoxAppAirconAuto *get_aircon_auto() { return &this->airconAuto_; }
  TrumaiNetBoxAppAirconManual *get_aircon_manual() { return &this->airconManual_; }
  TrumaiNetBoxAppClock *get_clock() { return &this->clock_; }
  TrumaiNetBoxAppConfig *get_config() { return &this->config_; }
  TrumaiNetBoxAppHeater *get_heater() { return &this->heater_; }
  TrumaiNetBoxAppTimer *get_timer() { return &this->timer_; }

  uint32_t get_last_cp_plus_request() { return this->device_registered_.load(std::memory_order_relaxed); }

  // Experimental: raw bytes 0 and 1 of LIN PID 0x22 (Combi 4 bus only, issue #25).
  // Callback runs in the main loop and only when one of the two bytes changed.
  void add_on_status_2_callback(std::function<void(uint8_t byte0, uint8_t byte1)> callback) {
    this->status_2_callback_.add(std::move(callback));
  }

#ifdef USE_TIME
  void set_time(time::RealTimeClock *time) { time_ = time; }
  time::RealTimeClock *get_time() const { return time_; }
#endif  // USE_TIME

 protected:
  // Truma CP Plus needs init (reset). This device is not registered.
  // Accessed from both the UART ISR/task and the main loop — must be atomic.
  std::atomic<uint32_t> device_registered_{0};
  std::atomic<uint32_t> init_requested_{0};
  std::atomic<uint32_t> init_received_{0};
  uint8_t message_counter = 1;

  // Truma heater connected to CP Plus.
  TRUMA_COMPANY company_ = TRUMA_COMPANY::TRUMA;
  // Written from uartEventTask_, read from main loop — must be atomic.
  std::atomic<TRUMA_DEVICE> heater_device_{TRUMA_DEVICE::UNKNOWN};
  std::atomic<TRUMA_DEVICE> aircon_device_{TRUMA_DEVICE::UNKNOWN};

  TrumaiNetBoxAppAirconAuto airconAuto_;
  TrumaiNetBoxAppAirconManual airconManual_;
  TrumaiNetBoxAppClock clock_;
  TrumaiNetBoxAppConfig config_;
  TrumaiNetBoxAppHeater heater_;
  TrumaiNetBoxAppTimer timer_;

  // last time CP plus was informed I got an update msg.
  std::atomic<uint32_t> update_time_{0};

  // PID 0x22 bytes 0 (low) and 1 (high). Written from uartEventTask_, read from main loop.
  std::atomic<uint16_t> status_2_raw_{0};
  std::atomic<bool> status_2_updated_{false};
  // Main loop only.
  bool status_2_published_{false};
  uint16_t status_2_last_published_{0};
  CallbackManager<void(uint8_t, uint8_t)> status_2_callback_{};

#ifdef USE_TIME
  time::RealTimeClock *time_ = nullptr;

  // Mark if the initial clock sync was done.
  bool update_status_clock_done = false;
#endif  // USE_TIME

  bool answer_lin_order_(const uint8_t pid) override;
  void lin_message_received_(const uint8_t pid, const uint8_t *message, uint8_t length) override;
  void publish_status_2_();

  bool lin_read_field_by_identifier_(uint8_t identifier, std::array<uint8_t, 5> *response) override;
  const uint8_t *lin_multiframe_received(const uint8_t *message, const uint8_t message_len,
                                          uint8_t *return_len) override;

  bool has_update_to_submit_();
};

}  // namespace truma_inetbox
}  // namespace esphome
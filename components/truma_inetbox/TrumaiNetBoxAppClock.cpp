#include "TrumaiNetBoxAppClock.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppClock";

// Verified on CP-Plus C3.00.00: the clock write (0A 14) is ignored and the panel stops serving the
// iNet Box until the next cycle. The write is only sent once the CP-Plus is known and is not a C3.
static constexpr uint8_t CPPLUS_C3_SOFTWARE_MAJOR = 0x03;

void TrumaiNetBoxAppClock::dump_data() const {
  ESP_LOGD(TAG, "StatusFrameClock %02d:%02d:%02d", this->data_.clock_hour, this->data_.clock_minute,
           this->data_.clock_second);
}

#ifdef USE_TIME
bool TrumaiNetBoxAppClock::action_write_time() {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }

  const uint8_t cpplus_major = this->parent_->get_cpplus_software_major();
  if (cpplus_major == 0) {
    ESP_LOGW(TAG, "CP Plus software unknown, not syncing the clock.");
    return false;
  }
  if (cpplus_major == CPPLUS_C3_SOFTWARE_MAJOR) {
    ESP_LOGW(TAG, "CP Plus C3 does not accept a clock write, not syncing the clock.");
    return false;
  }

  if (this->parent_->get_time() == nullptr) {
    ESP_LOGW(TAG, "Missing system time component.");
    return false;
  }

  auto now = this->parent_->get_time()->now();
  if (!now.is_valid()) {
    ESP_LOGW(TAG, "Invalid system time, not syncing to CP Plus.");
    return false;
  }

  // The behaviour of this method is special.
  // Just an update is marked. The actual package is prepared when CP Plus asks for the data in the
  // `lin_multiframe_received` method.
  this->update_submit();
  return true;
}

void TrumaiNetBoxAppClock::create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) {
  if (this->parent_->get_time() == nullptr) {
    *response_len = 0;
    this->update_status_unsubmitted_.store(false);
    return;
  }
  ESP_LOGD(TAG, "Requested read: Sending clock update");
  auto now = this->parent_->get_time()->now();

  status_frame_create_empty(response, STATUS_FRAME_CLOCK_RESPONSE, sizeof(StatusFrameClock), command_counter);

  response->clock.clock_hour = now.hour;
  response->clock.clock_minute = now.minute;
  response->clock.clock_second = now.second;
  response->clock.display_1 = 0x1;
  response->clock.display_2 = 0x1;
  response->clock.clock_mode = this->data_.clock_mode;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameClock);
  this->update_status_unsubmitted_.store(false);
}

#endif  // USE_TIME

}  // namespace truma_inetbox
}  // namespace esphome
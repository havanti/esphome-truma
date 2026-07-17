#include "TrumaCpPlusBinarySensor.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.cpplus_binary_sensor";

static constexpr uint32_t CP_PLUS_TIMEOUT_US = 90 * 1000 * 1000;  // 90 seconds without CP Plus request = disconnected

void TrumaCpPlusBinarySensor::update() {
  if (this->parent_->get_lin_bus_fault() || (this->parent_->get_last_cp_plus_request() == 0)) {
    this->publish_state(false);
    return;
  }
  // Subtraction idiom is wraparound-safe for the 32-bit micros() counter (overflows every ~71 min).
  const uint32_t since_last_request = micros() - this->parent_->get_last_cp_plus_request();
  this->publish_state(since_last_request < CP_PLUS_TIMEOUT_US);
}

void TrumaCpPlusBinarySensor::dump_config() { LOG_BINARY_SENSOR("", "Truma CP Plus Binary Sensor", this); }
}  // namespace truma_inetbox
}  // namespace esphome
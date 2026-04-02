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
  const auto timeout = this->parent_->get_last_cp_plus_request() + CP_PLUS_TIMEOUT_US;
  this->publish_state(micros() < timeout);
}

void TrumaCpPlusBinarySensor::dump_config() { LOG_BINARY_SENSOR("", "Truma CP Plus Binary Sensor", this); }
}  // namespace truma_inetbox
}  // namespace esphome
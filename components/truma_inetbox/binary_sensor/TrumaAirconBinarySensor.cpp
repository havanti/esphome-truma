#include "TrumaAirconBinarySensor.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_binary_sensor";

void TrumaAirconBinarySensor::setup() {
  this->parent_->get_aircon_auto()->add_on_message_callback([this](const StatusFrameAirconAuto *status) {
    switch (this->type_) {
      case TRUMA_BINARY_SENSOR_TYPE::AIRCON_AUTO_ACTIVE:
        // The CP Plus reports the AUTO target while AUTO is on and 0 after switching it off (issue #28 logs).
        this->publish_state(status->target_temp_aircon_auto != TargetTemp::TARGET_TEMP_OFF);
        break;
      default:
        break;
    }
  });
}

void TrumaAirconBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Truma Aircon Binary Sensor", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}
}  // namespace truma_inetbox
}  // namespace esphome

#include "TrumaAirconAutoNumber.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_auto_number";

void TrumaAirconAutoNumber::setup() {
  this->parent_->get_aircon_auto()->add_on_message_callback([this](const StatusFrameAirconAuto *status) {
    switch (this->type_) {
      case TRUMA_NUMBER_TYPE::AIRCON_AUTO_TEMPERATURE:
        this->publish_state(temp_code_to_decimal(status->target_temp_aircon_auto, 0));
        break;
      default:
        break;
    }
  });
}

void TrumaAirconAutoNumber::control(float value) {
  switch (this->type_) {
    case TRUMA_NUMBER_TYPE::AIRCON_AUTO_TEMPERATURE:
      this->parent_->get_aircon_auto()->action_set_temp(static_cast<uint8_t>(value));
      break;
    default:
      break;
  }
}

void TrumaAirconAutoNumber::dump_config() {
  LOG_NUMBER("", "Truma Aircon Auto Number", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}
}  // namespace truma_inetbox
}  // namespace esphome

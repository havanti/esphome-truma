#include "TrumaAirconAutoClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_auto_climate";

static constexpr float AIRCON_AUTO_DEFAULT_TEMPERATURE = 22.0f;
static constexpr uint8_t AIRCON_AUTO_OFF = 0;
static constexpr float AIRCON_AUTO_MIN_TEMPERATURE = 16.0f;
static constexpr float AIRCON_AUTO_MAX_TEMPERATURE = 31.0f;
static constexpr float AIRCON_AUTO_TEMPERATURE_STEP = 1.0f;

void TrumaAirconAutoClimate::setup() {
  this->parent_->get_aircon_auto()->add_on_message_callback([this](const StatusFrameAirconAuto *status) {
    // The CP Plus reports the AUTO target while AUTO is on and 0 after switching it off (issue #28 logs).
    this->target_temperature = temp_code_to_decimal(status->target_temp_aircon_auto);
    this->current_temperature = temp_code_to_decimal(status->current_temp_room);
    this->mode = (status->target_temp_aircon_auto == TargetTemp::TARGET_TEMP_OFF) ? climate::CLIMATE_MODE_OFF
                                                                                   : climate::CLIMATE_MODE_AUTO;
    this->publish_state();
  });
}

void TrumaAirconAutoClimate::dump_config() { LOG_CLIMATE(TAG, "Truma Aircon Auto Climate", this); }

void TrumaAirconAutoClimate::control(const climate::ClimateCall &call) {
  auto status = this->parent_->get_aircon_auto()->get_status();
  float temp = temp_code_to_decimal(status->target_temp_aircon_auto, AIRCON_AUTO_DEFAULT_TEMPERATURE);

  if (call.get_target_temperature().has_value()) {
    temp = *call.get_target_temperature();
  }

  if (call.get_mode().has_value() && *call.get_mode() == climate::CLIMATE_MODE_OFF) {
    temp = AIRCON_AUTO_OFF;
  }

  this->parent_->get_aircon_auto()->action_set_temp(static_cast<uint8_t>(temp));
}

climate::ClimateTraits TrumaAirconAutoClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_AUTO});
  traits.set_visual_min_temperature(AIRCON_AUTO_MIN_TEMPERATURE);
  traits.set_visual_max_temperature(AIRCON_AUTO_MAX_TEMPERATURE);
  traits.set_visual_temperature_step(AIRCON_AUTO_TEMPERATURE_STEP);
  return traits;
}

}  // namespace truma_inetbox
}  // namespace esphome

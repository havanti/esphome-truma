#include "TrumaWaterClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.water_climate";
// Water target range on the CP Plus.
static constexpr float WATER_TEMPERATURE_MIN = 40;
static constexpr float WATER_TEMPERATURE_MAX = 80;
static constexpr float WATER_TEMPERATURE_STEP = 20;
// The CP Plus reports the water temperature in whole degrees; Home Assistant MQTT discovery accepts precision values
// of 0.1, 0.5 or 1.
static constexpr float WATER_CURRENT_TEMPERATURE_STEP = 1;

void TrumaWaterClimate::setup() {
  this->parent_->get_heater()->add_on_message_callback([this](const StatusFrameHeater *status_heater) {
    // Publish updated state
    this->target_temperature = water_temp_200_fix(temp_code_to_decimal(status_heater->target_temp_water));
    this->current_temperature = temp_code_to_decimal(status_heater->current_temp_water);
    this->mode = (status_heater->target_temp_water == TargetTemp::TARGET_TEMP_OFF) ? climate::CLIMATE_MODE_OFF
                                                                                   : climate::CLIMATE_MODE_HEAT;
    this->publish_state();
  });
}

void TrumaWaterClimate::dump_config() { LOG_CLIMATE(TAG, "Truma Water Climate", this); }

void TrumaWaterClimate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    this->parent_->get_heater()->action_heater_water(static_cast<uint8_t>(temp));
  }

  if (call.get_mode().has_value()) {
    climate::ClimateMode mode = *call.get_mode();
    switch (mode) {
      case climate::CLIMATE_MODE_HEAT:
        // A target in the same call or a pending one from a previous call wins over the switch-on default.
        if (this->parent_->get_heater()->pending_target_temp_water() == TargetTemp::TARGET_TEMP_OFF &&
            !call.get_target_temperature().has_value()) {
          this->parent_->get_heater()->action_heater_water(WATER_SWITCH_ON_TEMPERATURE);
        }
        break;
      default:
        this->parent_->get_heater()->action_heater_water(0);
        break;
    }
  }
}

climate::ClimateTraits TrumaWaterClimate::traits() {
  // The capabilities of the climate device
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
  traits.set_visual_min_temperature(WATER_TEMPERATURE_MIN);
  traits.set_visual_max_temperature(WATER_TEMPERATURE_MAX);
  traits.set_visual_target_temperature_step(WATER_TEMPERATURE_STEP);
  traits.set_visual_current_temperature_step(WATER_CURRENT_TEMPERATURE_STEP);
  return traits;
}
}  // namespace truma_inetbox
}  // namespace esphome
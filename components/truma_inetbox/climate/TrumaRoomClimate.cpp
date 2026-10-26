#include "TrumaRoomClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.room_climate";
// Room target range on the CP Plus.
static constexpr float ROOM_TEMPERATURE_MIN = 5;
static constexpr float ROOM_TEMPERATURE_MAX = 30;
static constexpr float ROOM_TEMPERATURE_STEP = 1;
// Home Assistant takes the current step as precision: the room temperature comes in tenths.
static constexpr float ROOM_CURRENT_TEMPERATURE_STEP = 0.1;

void TrumaRoomClimate::setup() {
  this->parent_->get_heater()->add_on_message_callback([this](const StatusFrameHeater *status_heater) {
    // Publish updated state
    this->target_temperature = temp_code_to_decimal(status_heater->target_temp_room);
    this->current_temperature = temp_code_to_decimal(status_heater->current_temp_room);
    this->mode = std::isnan(this->target_temperature) ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;

    switch (status_heater->heating_mode) {
      case HeatingMode::HEATING_MODE_ECO:
      case HeatingMode::HEATING_MODE_VARIO_HEAT_NIGHT:
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
      case HeatingMode::HEATING_MODE_HIGH:
      case HeatingMode::HEATING_MODE_VARIO_HEAT_AUTO:
        this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
        break;
      case HeatingMode::HEATING_MODE_BOOST:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
      default:
        this->fan_mode = climate::CLIMATE_FAN_OFF;
        break;
    }
    this->publish_state();
  });
}

void TrumaRoomClimate::dump_config() { LOG_CLIMATE(TAG, "Truma Room Climate", this); }

void TrumaRoomClimate::control(const climate::ClimateCall &call) {
  auto heater = this->parent_->get_heater();
  if (call.get_target_temperature().has_value() && !call.get_fan_mode().has_value()) {
    float temp = *call.get_target_temperature();
    heater->action_heater_room(static_cast<uint8_t>(temp));
  }

  if (call.get_mode().has_value()) {
    // User requested mode change
    climate::ClimateMode mode = *call.get_mode();
    switch (mode) {
      case climate::CLIMATE_MODE_HEAT:
        // A target in the same call or a pending one from a previous call wins over the switch-on default.
        if (heater->pending_target_temp_room() == TargetTemp::TARGET_TEMP_OFF &&
            !call.get_target_temperature().has_value()) {
          heater->action_heater_room(ROOM_SWITCH_ON_TEMPERATURE);
        }
        break;
      default:
        heater->action_heater_room(0);
        break;
    }
  }

  if (call.get_fan_mode().has_value()) {
    auto fan_mode = *call.get_fan_mode();
    float temp = temp_code_to_decimal(heater->pending_target_temp_room(), 0);
    if (call.get_target_temperature().has_value()) {
      temp = *call.get_target_temperature();
    }
    switch (fan_mode) {
      case climate::CLIMATE_FAN_LOW:
      case climate::CLIMATE_FAN_MEDIUM:
      case climate::CLIMATE_FAN_HIGH:
        if (temp < ROOM_SWITCH_ON_TEMPERATURE) {
          temp = ROOM_SWITCH_ON_TEMPERATURE;
        }
        break;
      default:
        break;
    }
    // LOW/MEDIUM are Eco/High on a Combi and Night/Auto on a Vario Heat, like the fan mode select.
    switch (fan_mode) {
      case climate::CLIMATE_FAN_LOW:
        heater->action_heater_room(static_cast<uint8_t>(temp), heater->fan_mode_heating_mode(false));
        break;
      case climate::CLIMATE_FAN_MEDIUM:
        heater->action_heater_room(static_cast<uint8_t>(temp), heater->fan_mode_heating_mode(true));
        break;
      case climate::CLIMATE_FAN_HIGH:
        heater->action_heater_room(static_cast<uint8_t>(temp), HeatingMode::HEATING_MODE_BOOST);
        break;
      default:
        heater->action_heater_room(0);
        break;
    }
  }

}

climate::ClimateTraits TrumaRoomClimate::traits() {
  // The capabilities of the climate device
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
  traits.set_supported_fan_modes({{
      climate::CLIMATE_FAN_OFF,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
  }});
  traits.set_visual_min_temperature(ROOM_TEMPERATURE_MIN);
  traits.set_visual_max_temperature(ROOM_TEMPERATURE_MAX);
  traits.set_visual_target_temperature_step(ROOM_TEMPERATURE_STEP);
  traits.set_visual_current_temperature_step(ROOM_CURRENT_TEMPERATURE_STEP);
  return traits;
}
}  // namespace truma_inetbox
}  // namespace esphome
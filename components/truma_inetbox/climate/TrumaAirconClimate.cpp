#include "TrumaAirconClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_climate";
// Aventa target range on the CP Plus.
static constexpr float AIRCON_TEMPERATURE_MIN = 16;
static constexpr float AIRCON_TEMPERATURE_MAX = 31;
static constexpr float AIRCON_TEMPERATURE_STEP = 1;
// Home Assistant takes the current step as precision: the aircon temperature comes in tenths.
static constexpr float AIRCON_CURRENT_TEMPERATURE_STEP = 0.1;

void TrumaAirconClimate::setup() {
  this->parent_->get_aircon_manual()->add_on_message_callback([this](const StatusFrameAirconManual *status) {
    this->target_temperature = temp_code_to_decimal(status->target_temp_aircon);
    this->current_temperature = temp_code_to_decimal(status->current_temp_aircon);
    this->mode = this->aircon_mode_to_climate_mode_(status->mode);
    this->fan_mode = this->vent_mode_to_fan_mode_(status->vent_mode);
    this->publish_state();
  });
}

void TrumaAirconClimate::dump_config() { LOG_CLIMATE(TAG, "Truma Aircon Climate", this); }

void TrumaAirconClimate::control(const climate::ClimateCall &call) {
  // Every field goes through its own action. Each one starts from the pending update, so a value
  // from a previous call is kept until the CP Plus has confirmed it.
  auto aircon = this->parent_->get_aircon_manual();
  bool switched_off = false;
  if (call.get_mode().has_value()) {
    AirconMode mode = this->climate_mode_to_aircon_mode_(*call.get_mode());
    aircon->action_set_mode(mode);
    switched_off = mode == AirconMode::AIRCON_MODE_OFF;
  }
  // A vent mode would switch the aircon back on.
  if (call.get_fan_mode().has_value() && !switched_off) {
    aircon->action_set_vent_mode(this->fan_mode_to_vent_mode_(*call.get_fan_mode()));
  }
  if (call.get_target_temperature().has_value()) {
    aircon->action_set_temp(static_cast<uint8_t>(*call.get_target_temperature()));
  }
}

climate::ClimateTraits TrumaAirconClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(esphome::climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);

  for (auto mode : this->supported_modes_) {
    traits.add_supported_mode(mode);
  }

  traits.set_supported_fan_modes({{
      climate::CLIMATE_FAN_AUTO,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
  }});

  traits.set_visual_min_temperature(AIRCON_TEMPERATURE_MIN);
  traits.set_visual_max_temperature(AIRCON_TEMPERATURE_MAX);
  traits.set_visual_target_temperature_step(AIRCON_TEMPERATURE_STEP);
  traits.set_visual_current_temperature_step(AIRCON_CURRENT_TEMPERATURE_STEP);
  return traits;
}

void TrumaAirconClimate::set_supported_modes(const std::set<climate::ClimateMode> &modes) {
  this->supported_modes_ = modes;
}

climate::ClimateMode TrumaAirconClimate::aircon_mode_to_climate_mode_(AirconMode mode) {
  switch (mode) {
    case AirconMode::AIRCON_MODE_COOLING:
      return climate::CLIMATE_MODE_COOL;
    case AirconMode::AIRCON_MODE_HEATING:
      return climate::CLIMATE_MODE_HEAT;
    case AirconMode::AIRCON_MODE_AUTO:
      return climate::CLIMATE_MODE_HEAT_COOL;
    case AirconMode::AIRCON_MODE_VENTILATION:
      return climate::CLIMATE_MODE_FAN_ONLY;
    case AirconMode::AIRCON_MODE_OFF:
    default:
      return climate::CLIMATE_MODE_OFF;
  }
}

AirconMode TrumaAirconClimate::climate_mode_to_aircon_mode_(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_COOL:
      return AirconMode::AIRCON_MODE_COOLING;
    case climate::CLIMATE_MODE_HEAT:
      return AirconMode::AIRCON_MODE_HEATING;
    case climate::CLIMATE_MODE_HEAT_COOL:
    case climate::CLIMATE_MODE_AUTO:
      return AirconMode::AIRCON_MODE_AUTO;
    case climate::CLIMATE_MODE_FAN_ONLY:
      return AirconMode::AIRCON_MODE_VENTILATION;
    case climate::CLIMATE_MODE_OFF:
    default:
      return AirconMode::AIRCON_MODE_OFF;
  }
}

climate::ClimateFanMode TrumaAirconClimate::vent_mode_to_fan_mode_(AirconVentMode vent_mode) {
  switch (vent_mode) {
    case AirconVentMode::AIRCON_VENT_AUTO:
      return climate::CLIMATE_FAN_AUTO;
    case AirconVentMode::AIRCON_VENT_LOW:
    case AirconVentMode::AIRCON_VENT_NIGHT:
      return climate::CLIMATE_FAN_LOW;
    case AirconVentMode::AIRCON_VENT_MID:
      return climate::CLIMATE_FAN_MEDIUM;
    case AirconVentMode::AIRCON_VENT_HIGH:
      return climate::CLIMATE_FAN_HIGH;
    default:
      return climate::CLIMATE_FAN_LOW;
  }
}

AirconVentMode TrumaAirconClimate::fan_mode_to_vent_mode_(climate::ClimateFanMode fan_mode) {
  switch (fan_mode) {
    case climate::CLIMATE_FAN_AUTO:
      return AirconVentMode::AIRCON_VENT_AUTO;
    case climate::CLIMATE_FAN_LOW:
      return AirconVentMode::AIRCON_VENT_LOW;
    case climate::CLIMATE_FAN_MEDIUM:
      return AirconVentMode::AIRCON_VENT_MID;
    case climate::CLIMATE_FAN_HIGH:
      return AirconVentMode::AIRCON_VENT_HIGH;
    default:
      return AirconVentMode::AIRCON_VENT_LOW;
  }
}

}  // namespace truma_inetbox
}  // namespace esphome

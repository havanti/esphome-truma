#include "TrumaAirconClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_climate";

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
  auto status = this->parent_->get_aircon_manual()->get_status();
  float temp = temp_code_to_decimal(status->target_temp_aircon, 22);
  AirconMode mode = status->mode;
  AirconVentMode vent_mode = status->vent_mode;

  if (call.get_target_temperature().has_value()) {
    temp = *call.get_target_temperature();
  }

  if (call.get_mode().has_value()) {
    climate::ClimateMode climate_mode = *call.get_mode();
    mode = this->climate_mode_to_aircon_mode_(climate_mode);
  }

  if (call.get_fan_mode().has_value()) {
    climate::ClimateFanMode fan_mode = *call.get_fan_mode();
    vent_mode = this->fan_mode_to_vent_mode_(fan_mode);
  }

  this->parent_->get_aircon_manual()->action_aircon_manual(static_cast<uint8_t>(temp), mode, vent_mode);
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

  traits.set_visual_min_temperature(this->visual_min_temperature_);
  traits.set_visual_max_temperature(this->visual_max_temperature_);
  traits.set_visual_temperature_step(this->visual_temperature_step_);
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

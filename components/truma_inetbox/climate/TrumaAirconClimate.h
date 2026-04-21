#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/core/component.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"
#include <set>

namespace esphome {
namespace truma_inetbox {

class TrumaAirconClimate : public Component, public climate::Climate, public Parented<TrumaiNetBoxApp> {
 public:
  void dump_config() override;
  void control(const climate::ClimateCall &call) override;
  void setup() override;
  climate::ClimateTraits traits() override;

  void set_visual_min_temperature(float value) { this->visual_min_temperature_ = value; }
  void set_visual_max_temperature(float value) { this->visual_max_temperature_ = value; }
  void set_visual_temperature_step(float value) { this->visual_temperature_step_ = value; }

  void set_supported_modes(const std::set<climate::ClimateMode> &modes);

 protected:
  std::set<esphome::climate::ClimateMode> supported_modes_;
  float visual_min_temperature_{16.0};
  float visual_max_temperature_{31.0};
  float visual_temperature_step_{1.0};

  climate::ClimateMode aircon_mode_to_climate_mode_(AirconMode mode);
  AirconMode climate_mode_to_aircon_mode_(climate::ClimateMode mode);
  climate::ClimateFanMode vent_mode_to_fan_mode_(AirconVentMode vent_mode);
  AirconVentMode fan_mode_to_vent_mode_(climate::ClimateFanMode fan_mode);
};

}  // namespace truma_inetbox
}  // namespace esphome

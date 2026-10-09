#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

// AUTO on the CP Plus (heater and aircon together). Experimental, writing is not confirmed on hardware.
class TrumaAirconAutoClimate : public Component, public climate::Climate, public Parented<TrumaiNetBoxApp> {
 public:
  void setup() override;

  void dump_config() override;

  void control(const climate::ClimateCall &call) override;

  climate::ClimateTraits traits() override;
};
}  // namespace truma_inetbox
}  // namespace esphome

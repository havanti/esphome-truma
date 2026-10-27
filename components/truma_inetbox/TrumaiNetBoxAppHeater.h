#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppHeater : public TrumaStausFrameResponseStorage<StatusFrameHeater, StatusFrameHeaterResponse> {
 public:
  StatusFrameHeaterResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) override;
  void dump_data() const override;
  bool can_update() override;
  // LIN event task.
  void set_status(StatusFrameHeater val) override;

  // Heating mode for the fan-mode select: Vario Heat modes only on a Vario Heat heater (H3, TRUMA_DEVICE
  // HEATER_VARIO); every Combi uses ECO/HIGH. The Combi 6 Gas heater H5.00.00 reports 0x05, which
  // TRUMA_DEVICE also names CPPLUS_VARIO.
  HeatingMode fan_mode_heating_mode(bool high) const;

  // Main loop: the last status reports a heater error (HEATER_HAS_ERROR, "manual reset required").
  bool has_error() const;

  // Main loop: the target the CP Plus will see next — the pending update while one is prepared or
  // not yet confirmed by a status frame, otherwise the last status.
  TargetTemp pending_target_temp_room() const;
  TargetTemp pending_target_temp_water() const;

  bool action_heater_room(uint8_t temperature, HeatingMode mode = HeatingMode::HEATING_MODE_OFF);
  bool action_heater_water(uint8_t temperature);
  bool action_heater_water(TargetTemp temperature);
  bool action_heater_electric_power_level(uint16_t value);
  bool action_heater_energy_mix(EnergyMix energy_mix,
                                ElectricPowerLevel el_power_level = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0);
};

}  // namespace truma_inetbox
}  // namespace esphome
#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppTimer : public TrumaStausFrameResponseStorage<StatusFrameTimer, StatusFrameTimerResponse> {
 public:
  static constexpr uint16_t MINUTES_PER_DAY = 24 * 60;
  StatusFrameTimerResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) override;
  void dump_data() const override;
  
  bool action_timer_disable();
  bool action_timer_activate(uint16_t start, uint16_t stop, uint8_t room_temperature,
                             HeatingMode mode = HeatingMode::HEATING_MODE_OFF, uint8_t water_temperature = 0,
                             EnergyMix energy_mix = EnergyMix::ENERGY_MIX_NONE,
                             ElectricPowerLevel el_power_level = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0);
};

}  // namespace truma_inetbox
}  // namespace esphome
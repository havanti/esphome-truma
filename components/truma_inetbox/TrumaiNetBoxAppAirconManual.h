#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppAirconManual
    : public TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse> {
 public:
  StatusFrameAirconManualResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) override;
  void dump_data() const override;
  bool can_update() override;

  // Rounded to whole degrees; below 16 sets the target off.
  bool action_set_temp(float temperature);
  bool action_set_mode(AirconMode mode);
  bool action_set_vent_mode(AirconVentMode vent_mode);
  bool action_set_light(uint8_t level);
};

}  // namespace truma_inetbox
}  // namespace esphome
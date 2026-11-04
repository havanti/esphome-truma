#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppAirconAuto
    : public TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse> {
 public:
  StatusFrameAirconAutoResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) override;
  void set_status(StatusFrameAirconAuto val) override;
  void dump_data() const override;
  bool can_update() override;

  // Main loop: the AUTO target the CP Plus will see next — the pending update while one is prepared or not yet
  // confirmed by a status frame, otherwise the last status.
  TargetTemp pending_target_temp() const;

  // Sets the AUTO target on the CP Plus, rounded to whole degrees; below 18 switches AUTO off.
  bool action_set_temp(float temperature);
};

}  // namespace truma_inetbox
}  // namespace esphome

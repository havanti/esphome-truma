#include "TrumaiNetBoxAppAirconAuto.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppAirconAuto";

StatusFrameAirconAutoResponse *TrumaiNetBoxAppAirconAuto::update_prepare() {
  if (this->update_status_prepared_.load() || this->update_status_stale_.load()) {
    return &this->update_status_;
  }

  // Only the AUTO fields: create_update_data adds the heater fields when the frame goes out.
  this->update_status_ = {};
  this->update_status_.unknown_02 = this->data_.unknown_02;
  this->update_status_.unknown_04 = this->data_.unknown_04;
  this->update_status_.target_temp_aircon_auto = this->data_.target_temp_aircon_auto;

  this->update_status_prepared_.store(true);
  return &this->update_status_;
}

void TrumaiNetBoxAppAirconAuto::create_update_data(StatusFrame *response, uint8_t *response_len,
                                                   uint8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_AIRCON_AUTO_RESPONSE, sizeof(StatusFrameAirconAutoResponse),
                            command_counter);

  response->airconAutoResponse.unknown_02 = this->update_status_.unknown_02;
  response->airconAutoResponse.unknown_04 = this->update_status_.unknown_04;
  response->airconAutoResponse.target_temp_aircon_auto = this->update_status_.target_temp_aircon_auto;

  // The heater fields go along as the heater will have them, otherwise the write could undo a heater update the last
  // AUTO status does not show yet. An original iNet Box sends a just changed water target here before the CP Plus
  // reports it (issue #32 logs). Without a heater status they come from the last AUTO status.
  auto *heater = this->parent_->get_heater();
  if (heater->get_status_valid()) {
    const StatusFrameHeaterResponse pending = heater->pending_response();
    response->airconAutoResponse.energy_mix_a = pending.energy_mix_a;
    response->airconAutoResponse.energy_mix_b = pending.energy_mix_b;
    response->airconAutoResponse.el_power_level_a = pending.el_power_level_a;
    response->airconAutoResponse.target_temp_water = pending.target_temp_water;
    response->airconAutoResponse.el_power_level_b = pending.el_power_level_b;
  } else {
    response->airconAutoResponse.energy_mix_a = this->data_.energy_mix_a;
    response->airconAutoResponse.energy_mix_b = this->data_.energy_mix_b;
    response->airconAutoResponse.el_power_level_a = this->data_.el_power_level_a;
    response->airconAutoResponse.target_temp_water = this->data_.target_temp_water;
    response->airconAutoResponse.el_power_level_b = this->data_.el_power_level_b;
  }

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameAirconAutoResponse);

  TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse>::update_submitted();
}

void TrumaiNetBoxAppAirconAuto::set_status(StatusFrameAirconAuto val) {
  // Same as the heater status: 0x0AAA on both means no data, stored as 0, which reads as unknown.
  if (static_cast<uint16_t>(val.current_temp_room) == TEMP_CODE_NO_DATA &&
      static_cast<uint16_t>(val.current_temp_water) == TEMP_CODE_NO_DATA) {
    val.current_temp_room = TargetTemp::TARGET_TEMP_OFF;
    val.current_temp_water = TargetTemp::TARGET_TEMP_OFF;
  }
  TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse>::set_status(val);
}

void TrumaiNetBoxAppAirconAuto::dump_data() const {}

bool TrumaiNetBoxAppAirconAuto::can_update() {
  return TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse>::can_update() &&
         this->parent_->get_aircon_device() != TRUMA_DEVICE::UNKNOWN;
}

TargetTemp TrumaiNetBoxAppAirconAuto::pending_target_temp() const {
  if (this->update_status_prepared_.load() || this->update_status_stale_.load()) {
    return this->update_status_.target_temp_aircon_auto;
  }
  return this->data_.target_temp_aircon_auto;
}

bool TrumaiNetBoxAppAirconAuto::action_set_temp(float temperature) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma aircon AUTO.");
    return false;
  }

  auto update_data = this->update_prepare();
  update_data->target_temp_aircon_auto = decimal_to_aircon_auto_temp(temperature);

  this->update_submit();
  return true;
}

}  // namespace truma_inetbox
}  // namespace esphome

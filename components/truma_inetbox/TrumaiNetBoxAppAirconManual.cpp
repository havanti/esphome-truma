#include "TrumaiNetBoxAppAirconManual.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppAirconManual";

static bool is_valid_vent_mode(AirconVentMode vent_mode) {
  switch (vent_mode) {
    case AirconVentMode::AIRCON_VENT_LOW:
    case AirconVentMode::AIRCON_VENT_MID:
    case AirconVentMode::AIRCON_VENT_HIGH:
    case AirconVentMode::AIRCON_VENT_NIGHT:
    case AirconVentMode::AIRCON_VENT_AUTO:
      return true;
    default:
      return false;
  }
}

StatusFrameAirconManualResponse *TrumaiNetBoxAppAirconManual::update_prepare() {
  if (this->update_status_prepared_ || this->update_status_stale_) {
    return &this->update_status_;
  }

  this->update_status_ = {};
  this->update_status_.mode = this->data_.mode;
  this->update_status_.vent_mode = this->data_.vent_mode;
  this->update_status_.aircon_on = 0x01;  // Must be 1 for commands to be accepted
  this->update_status_.target_temp_aircon = this->data_.target_temp_aircon;

  if (this->update_status_.target_temp_aircon == TargetTemp::TARGET_TEMP_OFF ||
      static_cast<uint16_t>(this->update_status_.target_temp_aircon) == 0) {
    this->update_status_.target_temp_aircon = TargetTemp::TARGET_TEMP_22C;
  }

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

void TrumaiNetBoxAppAirconManual::create_update_data(StatusFrame *response, uint8_t *response_len,
                                                     uint8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_AIRCON_MANUAL_RESPONSE, sizeof(StatusFrameAirconManualResponse),
                            command_counter);

  // CP Plus C.04.05.02 reports vent mode 0xFF while the aircon is off (issue #28). Echoing it back
  // is rejected with ACK 0x02, so replace any unknown value before sending.
  AirconVentMode vent_mode = this->update_status_.vent_mode;
  if (!is_valid_vent_mode(vent_mode)) {
    vent_mode = this->update_status_.mode == AirconMode::AIRCON_MODE_AUTO ? AirconVentMode::AIRCON_VENT_AUTO
                                                                          : AirconVentMode::AIRCON_VENT_LOW;
    ESP_LOGD(TAG, "Unknown vent mode 0x%02X replaced by 0x%02X", (uint8_t) this->update_status_.vent_mode,
             (uint8_t) vent_mode);
  }

  response->airconManualResponse.mode = this->update_status_.mode;
  response->airconManualResponse.unknown_02 = 0x00;
  response->airconManualResponse.vent_mode = vent_mode;
  response->airconManualResponse.aircon_on = 0x01;  // Must always be 1
  response->airconManualResponse.target_temp_aircon = this->update_status_.target_temp_aircon;
  memset(response->airconManualResponse.padding, 0x00, sizeof(response->airconManualResponse.padding));
  // Echo the water target from the last aircon status frame, otherwise the write turns the water heater off.
  response->airconManualResponse.target_temp_water = this->data_.target_temp_water;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameAirconManualResponse);

  TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse>::update_submitted();
}

void TrumaiNetBoxAppAirconManual::dump_data() const {}

bool TrumaiNetBoxAppAirconManual::can_update() {
  return TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse>::can_update() &&
         this->parent_->get_aircon_device() != TRUMA_DEVICE::UNKNOWN;
}

bool TrumaiNetBoxAppAirconManual::action_set_temp(uint8_t temperature) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma aircon.");
    return false;
  }

  auto update_data = this->update_prepare();
  update_data->target_temp_aircon = decimal_to_aircon_manual_temp(temperature);

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppAirconManual::action_set_mode(AirconMode mode) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma aircon.");
    return false;
  }

  auto update_data = this->update_prepare();
  update_data->mode = mode;

  if (mode == AirconMode::AIRCON_MODE_OFF) {
    update_data->vent_mode = AirconVentMode::AIRCON_VENT_LOW;
  } else if (mode == AirconMode::AIRCON_MODE_AUTO) {
    // AUTO mode requires AUTO vent — protocol constraint
    update_data->vent_mode = AirconVentMode::AIRCON_VENT_AUTO;
  } else if (update_data->vent_mode == AirconVentMode::AIRCON_VENT_AUTO) {
    update_data->vent_mode = AirconVentMode::AIRCON_VENT_LOW;
  }

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppAirconManual::action_set_vent_mode(AirconVentMode vent_mode) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma aircon.");
    return false;
  }

  auto update_data = this->update_prepare();

  if (vent_mode == AirconVentMode::AIRCON_VENT_AUTO) {
    // AUTO vent only valid with AUTO mode — protocol constraint
    update_data->mode = AirconMode::AIRCON_MODE_AUTO;
  } else if (update_data->mode == AirconMode::AIRCON_MODE_OFF) {
    update_data->mode = AirconMode::AIRCON_MODE_VENTILATION;
  } else if (update_data->mode == AirconMode::AIRCON_MODE_AUTO) {
    update_data->mode = AirconMode::AIRCON_MODE_COOLING;
  }

  update_data->vent_mode = vent_mode;

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppAirconManual::action_aircon_manual(uint8_t temperature, AirconMode mode,
                                                        AirconVentMode vent_mode) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma aircon.");
    return false;
  }

  auto update_data = this->update_prepare();
  update_data->mode = mode;
  update_data->vent_mode = vent_mode;
  update_data->target_temp_aircon = decimal_to_aircon_manual_temp(temperature);

  if (mode == AirconMode::AIRCON_MODE_OFF) {
    update_data->vent_mode = AirconVentMode::AIRCON_VENT_LOW;
  } else if (mode == AirconMode::AIRCON_MODE_AUTO) {
    update_data->vent_mode = AirconVentMode::AIRCON_VENT_AUTO;
  } else if (update_data->vent_mode == AirconVentMode::AIRCON_VENT_AUTO) {
    update_data->vent_mode = AirconVentMode::AIRCON_VENT_LOW;
  }

  this->update_submit();
  return true;
}

}  // namespace truma_inetbox
}  // namespace esphome

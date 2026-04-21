#include "TrumaAirconSelect.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_select";

void TrumaAirconSelect::setup() {
  this->parent_->get_aircon_manual()->add_on_message_callback([this](const StatusFrameAirconManual *status) {
    switch (this->type_) {
      case TRUMA_SELECT_TYPE::AIRCON_MODE:
        switch (status->mode) {
          case AirconMode::AIRCON_MODE_VENTILATION:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_MODE::VENTILATION).value());
            break;
          case AirconMode::AIRCON_MODE_COOLING:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_MODE::COOLING).value());
            break;
          case AirconMode::AIRCON_MODE_HEATING:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_MODE::HEATING).value());
            break;
          case AirconMode::AIRCON_MODE_AUTO:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_MODE::AUTO).value());
            break;
          case AirconMode::AIRCON_MODE_OFF:
          default:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_MODE::OFF).value());
            break;
        }
        break;

      case TRUMA_SELECT_TYPE::AIRCON_VENT_MODE:
        switch (status->vent_mode) {
          case AirconVentMode::AIRCON_VENT_LOW:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_LOW).value());
            break;
          case AirconVentMode::AIRCON_VENT_MID:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_MID).value());
            break;
          case AirconVentMode::AIRCON_VENT_HIGH:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_HIGH).value());
            break;
          case AirconVentMode::AIRCON_VENT_NIGHT:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_NIGHT).value());
            break;
          case AirconVentMode::AIRCON_VENT_AUTO:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_AUTO).value());
            break;
          default:
            this->publish_state(this->at((size_t) TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_LOW).value());
            break;
        }
        break;

      default:
        break;
    }
  });
}

void TrumaAirconSelect::control(const std::string &value) {
  auto index = this->index_of(value);
  if (!index.has_value()) {
    return;
  }

  switch (this->type_) {
    case TRUMA_SELECT_TYPE::AIRCON_MODE:
      switch ((TRUMA_SELECT_TYPE_AIRCON_MODE) index.value()) {
        case TRUMA_SELECT_TYPE_AIRCON_MODE::OFF:
          this->parent_->get_aircon_manual()->action_set_mode(AirconMode::AIRCON_MODE_OFF);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_MODE::VENTILATION:
          this->parent_->get_aircon_manual()->action_set_mode(AirconMode::AIRCON_MODE_VENTILATION);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_MODE::COOLING:
          this->parent_->get_aircon_manual()->action_set_mode(AirconMode::AIRCON_MODE_COOLING);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_MODE::HEATING:
          this->parent_->get_aircon_manual()->action_set_mode(AirconMode::AIRCON_MODE_HEATING);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_MODE::AUTO:
          this->parent_->get_aircon_manual()->action_set_mode(AirconMode::AIRCON_MODE_AUTO);
          break;
        default:
          break;
      }
      break;

    case TRUMA_SELECT_TYPE::AIRCON_VENT_MODE:
      switch ((TRUMA_SELECT_TYPE_AIRCON_VENT_MODE) index.value()) {
        case TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_LOW:
          this->parent_->get_aircon_manual()->action_set_vent_mode(AirconVentMode::AIRCON_VENT_LOW);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_MID:
          this->parent_->get_aircon_manual()->action_set_vent_mode(AirconVentMode::AIRCON_VENT_MID);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_HIGH:
          this->parent_->get_aircon_manual()->action_set_vent_mode(AirconVentMode::AIRCON_VENT_HIGH);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_NIGHT:
          this->parent_->get_aircon_manual()->action_set_vent_mode(AirconVentMode::AIRCON_VENT_NIGHT);
          break;
        case TRUMA_SELECT_TYPE_AIRCON_VENT_MODE::VENT_AUTO:
          this->parent_->get_aircon_manual()->action_set_vent_mode(AirconVentMode::AIRCON_VENT_AUTO);
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }
}

void TrumaAirconSelect::dump_config() {
  LOG_SELECT("", "Truma Aircon Select", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}
}  // namespace truma_inetbox
}  // namespace esphome

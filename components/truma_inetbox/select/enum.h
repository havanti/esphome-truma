#pragma once

#include "esphome/core/log.h"

namespace esphome {
namespace truma_inetbox {

enum class TRUMA_SELECT_TYPE {
  UNKNOWN,

  HEATER_FAN_MODE,
  HEATER_ENERGY_MIX,
  AIRCON_MODE,
  AIRCON_VENT_MODE,
};

enum class TRUMA_SELECT_TYPE_HEATER_FAN_MODE {
  OFF = 0,
  ECO = 1,
  VARIO_HEAT_NIGHT = 1,
  COMBI_HIGH = 2,
  VARIO_HEAT_AUTO = 2,
  BOOST = 3,
};

enum class TRUMA_SELECT_TYPE_HEATER_ENERGY_MIX {
  GAS = 0,
  MIX_1 = 1,
  MIX_2 = 2,
  ELECTRIC_1 = 3,
  ELECTRIC_2 = 4,
};

enum class TRUMA_SELECT_TYPE_AIRCON_MODE {
  OFF = 0,
  VENTILATION = 1,
  COOLING = 2,
  HEATING = 3,
  AUTO = 4,
};

enum class TRUMA_SELECT_TYPE_AIRCON_VENT_MODE {
  VENT_LOW = 0,
  VENT_MID = 1,
  VENT_HIGH = 2,
  VENT_NIGHT = 3,
  VENT_AUTO = 4,
};

#ifdef ESPHOME_LOG_HAS_CONFIG
static const char *enum_to_c_str(const TRUMA_SELECT_TYPE val) {
  switch (val) {
    case TRUMA_SELECT_TYPE::HEATER_FAN_MODE:
      return "HEATER_FAN_MODE";
    case TRUMA_SELECT_TYPE::HEATER_ENERGY_MIX:
      return "HEATER_ENERGY_MIX";
    case TRUMA_SELECT_TYPE::AIRCON_MODE:
      return "AIRCON_MODE";
    case TRUMA_SELECT_TYPE::AIRCON_VENT_MODE:
      return "AIRCON_VENT_MODE";
    default:
      return "";
  }
}
#endif  // ESPHOME_LOG_HAS_CONFIG

}  // namespace truma_inetbox
}  // namespace esphome

#pragma once

#include "TrumaEnums.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {
// First byte is service identifier and to be ignored. Last three bytes can be `xFF` or `x00` (see
// <https://github.com/Fabian-Schmidt/esphome-truma_inetbox/issues/25>).
// `truma_message_header` and `alde_message_header` must have the same size!
inline constexpr std::array<uint8_t, 11> truma_message_header = {0x00, 0x00, 0x1F, 0x00, 0x1E, 0x00,
                                                                  0x00, 0x22, 0xFF, 0xFF, 0xFF};
inline constexpr std::array<uint8_t, 11> alde_message_header = {0x00, 0x00, 0x1F, 0x00, 0x1A, 0x00,
                                                                 0x00, 0x22, 0xFF, 0xFF, 0xFF};

// Targets the CP Plus uses when room or water heating is switched on without one.
inline constexpr uint8_t ROOM_SWITCH_ON_TEMPERATURE = 5;
inline constexpr uint8_t WATER_SWITCH_ON_TEMPERATURE = 40;

// Temperature codes (Kelvin x10) that mark a state instead of a reading.
// 0.0 °C: no data from the heater in a status frame, an idle frame in PID 0x16.
inline constexpr uint16_t TEMP_CODE_NO_DATA = 0x0AAA;
// 31.1 °C: room value of a Combi 6 H5.00.00 after a bus wake-up, in PID 0x16 and in the next heater status.
inline constexpr uint16_t TEMP_CODE_WAKE_UP_ROOM = 0x0BE1;

// AUTO range of the CP Plus with climate automatic (iNet app: 17.8-25 °C, default 22, 0 = off).
inline constexpr uint8_t AIRCON_AUTO_TEMPERATURE_MIN = 18;
inline constexpr uint8_t AIRCON_AUTO_TEMPERATURE_MAX = 25;
inline constexpr uint8_t AIRCON_AUTO_TEMPERATURE_DEFAULT = 22;

uint8_t addr_parity(const uint8_t pid);
uint8_t data_checksum(const uint8_t *message, uint8_t length, uint16_t sum);
float temp_code_to_decimal(uint16_t val, float zero = NAN);
float temp_code_to_decimal(TargetTemp val, float zero = NAN);
float water_temp_200_fix(float val);
TargetTemp decimal_to_temp(uint8_t val);
TargetTemp decimal_to_temp(float val);
TargetTemp decimal_to_room_temp(uint8_t val);
TargetTemp decimal_to_room_temp(float val);
TargetTemp decimal_to_aircon_manual_temp(uint8_t val);
TargetTemp decimal_to_aircon_manual_temp(float val);
TargetTemp decimal_to_aircon_auto_temp(uint8_t val);
TargetTemp decimal_to_aircon_auto_temp(float val);
TargetTemp decimal_to_water_temp(uint8_t val);
TargetTemp decimal_to_water_temp(float val);
const std::string operating_status_to_str(OperatingStatus val);
ElectricPowerLevel decimal_to_el_power_level(uint16_t val);

}  // namespace truma_inetbox
}  // namespace esphome
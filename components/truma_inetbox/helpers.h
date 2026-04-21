#pragma once

#include "TrumaEnums.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {
// First byte is service identifier and to be ignored. Last three bytes can be `xFF` or `x00` (see
// <https://github.com/Fabian-Schmidt/esphome-truma_inetbox/issues/25>).
// `truma_message_header` and `alde_message_header` must have the same size!
const std::array<uint8_t, 11> truma_message_header = {0x00, 0x00, 0x1F, 0x00, 0x1E, 0x00,
                                                       0x00, 0x22, 0xFF, 0xFF, 0xFF};
const std::array<uint8_t, 11> alde_message_header = {0x00, 0x00, 0x1F, 0x00, 0x1A, 0x00, 0x00, 0x22, 0xFF, 0xFF, 0xFF};

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
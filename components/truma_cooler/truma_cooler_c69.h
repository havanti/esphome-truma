#pragma once

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "truma_cooler.h"

namespace esphome {
namespace truma_cooler {

// ===========================================================================
// TrumaCoolerC69 — dual-zone Truma Cooler C69.
// Reverse-engineered from an HCI snoop of the official Truma app (app 2.7.3),
// cross-checked against a timed action log. Status notify layout
// (0xAA 0xC1 0xF2 0xA1 ...):
//   [4]  bit0 = device ON    (0x09 = on, 0x08 = off)
//   [5]  status bitfield: bit2 = turbo, bit3 = compressor IDLE (0 = running)
//   [6]  zone 1 interior temperature (signed int8, °C)
//   [7]  zone 1 setpoint echoed back (signed int8, °C)
//   [8]  zone 2 interior temperature (signed int8, °C)
//   [9]  zone 2 setpoint echoed back (signed int8, °C)
//   [10] fixed (0x0A observed) — meaning unconfirmed
//   [11] noisy value — NOT exposed (C44 treats it as ambient; unclear on C69)
//
// Power is global (no per-zone power in the protocol): the zone climates are
// COOL-only and a single master `power` switch owns on/off. Turbo is deliberately
// NOT implemented for the C69 — it does not work reliably even on the C44.
// ===========================================================================

// Byte [4] device-state mask.
static constexpr uint8_t C69_DEVICE_ON_MASK = 0x01;
// Byte [5] compressor: bit3 set = idle (0 = running). Confirmed on real C69
// hardware (issue #18): compressor spin-up observed with byte5=0x07, idle 0x0B.
static constexpr uint8_t C69_COMPRESSOR_IDLE_MASK = 0x08;

class TrumaCoolerC69 : public TrumaCooler {
 public:
  void set_climate_zone1(TrumaCoolerClimate *c) { climate_zone1_ = c; }
  void set_climate_zone2(TrumaCoolerClimate *c) { climate_zone2_ = c; }
  void set_temperature_zone1_sensor(sensor::Sensor *s) { temperature_zone1_sensor_ = s; }
  void set_temperature_zone2_sensor(sensor::Sensor *s) { temperature_zone2_sensor_ = s; }

  void set_zone_setpoint(uint8_t zone, float temp_celsius) override;

 protected:
  const char *model_name_() const override { return "C69"; }
  void handle_status_(const uint8_t *data) override;
  void reset_entities_() override;
  void apply_restored_states_() override;

  void publish_zone_(TrumaCoolerClimate *c, float interior, int8_t setpoint,
                     bool device_on, bool compressor_running);

  TrumaCoolerClimate *climate_zone1_{nullptr};
  TrumaCoolerClimate *climate_zone2_{nullptr};
  sensor::Sensor *temperature_zone1_sensor_{nullptr};
  sensor::Sensor *temperature_zone2_sensor_{nullptr};
};

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

#pragma once

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "truma_cooler.h"

namespace esphome {
namespace truma_cooler {

// ===========================================================================
// TrumaCoolerC44 — single-zone Truma Cooler C44.
// Status notify (0xAA 0xC1 0xF2 0xA0 ...):
//   [4]  0x01 = on / 0x00 = off              device state
//   [5]  0x01 = running / 0x0D = turbo        compressor state
//   [6]  interior temperature (signed int8, °C)
//   [7]  setpoint echoed back (signed int8, °C)
//   [11] ambient/outside temperature in 0.1 °C (signed int8)
// Decoding is by exact byte value — do NOT change it (differs from the C69,
// whose byte [5] is a bitfield). See truma_cooler_c69.* for that model.
// ===========================================================================

// Byte [4] device state.
static constexpr uint8_t C44_DEVICE_ON = 0x01;
// Byte [5] compressor state markers.
static constexpr uint8_t C44_COMPRESSOR_RUNNING = 0x01;
static constexpr uint8_t C44_COMPRESSOR_TURBO = 0x0D;
// Turbo auto-reset delay after device power-on.
static constexpr uint32_t C44_TURBO_RESET_DELAY_MS = 500;

class TrumaCoolerC44 : public TrumaCooler {
 public:
  void set_temperature_sensor(sensor::Sensor *s) { temperature_sensor_ = s; }
  void set_ambient_temperature_sensor(sensor::Sensor *s) { ambient_temperature_sensor_ = s; }
  void set_turbo_running_sensor(binary_sensor::BinarySensor *s) { turbo_running_sensor_ = s; }
  void set_climate(TrumaCoolerClimate *c) { climate_ = c; }
  void set_turbo_switch(TrumaCoolerSwitch *s) { turbo_switch_ = s; }

  void set_zone_setpoint(uint8_t zone, float temp_celsius) override;
  void set_turbo(bool state) override;

 protected:
  const char *model_name_() const override { return "C44"; }
  void handle_status_(const uint8_t *data) override;
  void reset_entities_() override;
  void apply_restored_states_() override;
  void post_power_(bool on) override;
  void on_disconnect_cleanup_() override;

  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  binary_sensor::BinarySensor *turbo_running_sensor_{nullptr};
  TrumaCoolerClimate *climate_{nullptr};
  TrumaCoolerSwitch *turbo_switch_{nullptr};
};

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

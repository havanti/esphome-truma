#pragma once

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include <atomic>

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace truma_cooler {

// ===========================================================================
// Shared protocol constants (identical across all Truma Cooler models)
// ===========================================================================
// Service UUID: 0xFFF0. Write characteristic Handle 0x0012, Notify Handle 0x0015.
// All frames are 16 bytes, header 0xAA 0xC1, checksum = (sum(byte[0..14]) + 1) % 256.
//
// Command frame (host -> device), 0xAA 0xC1 0xF1 ...:
//   [3]  0x01 = power ON (global; no per-zone power exists)
//   [4]  0x01 = turbo (C44 only, device must already be ON)
//   [7]  Zone 1 setpoint (signed int8, °C)
//   [8]  Zone 2 setpoint (signed int8, °C) — C69 only
//   [9]  Zone selector for setpoint writes: 0x01 = zone 1, 0x02 = zone 2
//
// Status notify (device -> host), 0xAA 0xC1 0xF2 ...:
//   Byte-level meaning differs per model — decoded in the model subclass.

// GATT handles — hardcoded from HCI snoop / GATT attribute table.
static constexpr uint16_t WRITE_HANDLE = 0x0012;
static constexpr uint16_t NOTIFY_HANDLE = 0x0015;
static constexpr uint16_t CCCD_HANDLE = NOTIFY_HANDLE + 1;  // 0x0016

// Setpoint range supported by the device (also exposed via climate traits).
static constexpr float SETPOINT_MIN_C = -22.0f;
static constexpr float SETPOINT_MAX_C = 10.0f;

// Protocol frame layout.
static constexpr uint8_t FRAME_LEN = 16;
static constexpr uint8_t HEADER_B0 = 0xAA;
static constexpr uint8_t HEADER_B1 = 0xC1;
static constexpr uint8_t RESPONSE_TYPE = 0xF2;

// Setpoint-write zone selector (command byte [9]).
static constexpr uint8_t ZONE1_SELECT = 0x01;
static constexpr uint8_t ZONE2_SELECT = 0x02;

// Polling cadence (fallback only — device sends unsolicited notifications every ~2 s).
static constexpr uint32_t POLL_INTERVAL_MS = 60000;

class TrumaCoolerClimate;

// ===========================================================================
// TrumaCooler — shared base: BLE plumbing, framing, power control.
// Model-specific status decoding and entity wiring live in the C44/C69
// subclasses (truma_cooler_c44.*, truma_cooler_c69.*).
// ===========================================================================
class TrumaCooler : public Component, public ble_client::BLEClientNode {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  // Shared entity setters (present on both models).
  void set_connected_sensor(binary_sensor::BinarySensor *s) { connected_sensor_ = s; }
  void set_device_on_sensor(binary_sensor::BinarySensor *s) { device_on_sensor_ = s; }
  void set_compressor_running_sensor(binary_sensor::BinarySensor *s) { compressor_running_sensor_ = s; }

  // Control API (called by the climate / switch entities).
  void set_mode(bool on);
  virtual void set_zone_setpoint(uint8_t zone, float temp_celsius) = 0;
  virtual void set_turbo(bool state) {}  // no-op by default; C44 overrides

  void send_poll();
  void send_command(const uint8_t *cmd, size_t len);

 protected:
  // Model hooks.
  virtual const char *model_name_() const = 0;
  virtual void handle_status_(const uint8_t *data) = 0;  // frame already validated
  virtual void reset_entities_() = 0;                    // baseline on (dis)connect
  virtual void apply_restored_states_() {}               // restore climate(s) from flash
  virtual void post_power_(bool on) {}                   // e.g. C44 turbo auto-reset
  virtual void on_disconnect_cleanup_() {}               // e.g. C44 cancel turbo timeout

  void parse_notification_(const uint8_t *data, uint16_t len);
  // Build and send an 0xF1 setpoint frame with the given payload bytes.
  void send_setpoint_frame_(uint8_t byte7, uint8_t byte8, uint8_t zone_select);
  static uint8_t calculate_checksum_(const uint8_t *data, size_t len);

  binary_sensor::BinarySensor *connected_sensor_{nullptr};
  binary_sensor::BinarySensor *device_on_sensor_{nullptr};
  binary_sensor::BinarySensor *compressor_running_sensor_{nullptr};

  // Written from BT task (gattc_event_handler), read from app task (loop/send_command).
  // std::atomic for cross-task safety — `volatile` does not guarantee atomicity.
  std::atomic<uint16_t> write_handle_{0};
  std::atomic<bool> connected_{false};
  std::atomic<bool> poll_enabled_{false};
  std::atomic<bool> device_is_on_{false};
  uint32_t last_poll_{0};
};

// Zone-aware climate. zone_ = 0 for the single-zone C44, 1 / 2 for the C69 zones.
class TrumaCoolerClimate : public climate::Climate, public Parented<TrumaCooler> {
 public:
  void set_zone(uint8_t zone) { zone_ = zone; }
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;
  // Restore last persisted mode/setpoint from flash so HA sees a stable entity
  // before the first BLE notification arrives after reboot.
  void apply_restored_state();

 protected:
  uint8_t zone_{0};
};

class TrumaCoolerSwitch : public switch_::Switch, public Parented<TrumaCooler> {
 protected:
  void write_state(bool state) override;
};

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

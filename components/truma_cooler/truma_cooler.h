#pragma once

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace truma_cooler {

// Protocol constants
// Service UUID: 0xFFF0 (PKOC / vendor)
// Write characteristic UUID: 0xFFF7 / Handle 0x0012
// Notify characteristic:     Handle 0x0015 (UUID unknown)
//
// Notification (response) format (16 bytes):
//   [0]  0xAA  Header
//   [1]  0xC1  Header
//   [2]  0xF2  Response type
//   [3]  0xA0  Fixed
//   [4]  0x01=on / 0x00=off  Device state
//   [5]  0x01=running / 0x0D=turbo running / 0x09=idle  Compressor state
//   [6]  Interior temperature in °C (signed int8_t)
//   [7]  Setpoint echoed back in °C (signed int8_t)
//   [8-10] Fixed
//   [11] Ambient/outside temperature in 0.1°C, signed int8_t (e.g. 0x2B = 43 = 4.3°C; 0xFF = -1 = -0.1°C)
//   [12-14] Fixed
//   [15] CHECKSUM = (sum(byte[0..14]) + 1) % 256
//
// Command format (16 bytes):
//   [0]  0xAA  Header
//   [1]  0xC1  Header
//   [2]  0xF0=poll / 0xF1=control  Command type
//   [3]  0x01=turn on / 0x00=other commands  Power-ON flag (NOT always zero!)
//   [4]  0x00 for most commands / 0x01=turbo (only when device is already ON)
//   [5-6] 0x00  Fixed
//   [7]  Setpoint in °C (signed int8_t)
//   [8]  0x00  Fixed
//   [9]  0x03=activate(ON) / 0x01=setpoint/off  Command flag
//        0x03 REQUIRED when turning ON — device ignores ON command with 0x01
//        0x01 used for setpoint changes and turning OFF
//   [10-14] 0x00  Fixed
//   [15] CHECKSUM = (sum(byte[0..14]) + 1) % 256

static constexpr uint16_t NOTIFY_HANDLE = 0x0015;

// Setpoint range supported by the device (also exposed via climate traits).
static constexpr float SETPOINT_MIN_C = -22.0f;
static constexpr float SETPOINT_MAX_C = 10.0f;

// Protocol frame layout.
static constexpr uint8_t FRAME_LEN = 16;
static constexpr uint8_t HEADER_B0 = 0xAA;
static constexpr uint8_t HEADER_B1 = 0xC1;
static constexpr uint8_t RESPONSE_TYPE = 0xF2;

// Byte [4] device state.
static constexpr uint8_t DEVICE_ON = 0x01;

// Byte [5] compressor state markers.
static constexpr uint8_t COMPRESSOR_RUNNING = 0x01;
static constexpr uint8_t COMPRESSOR_TURBO = 0x0D;

// Polling cadence and turbo auto-reset delay.
static constexpr uint32_t POLL_INTERVAL_MS = 60000;
static constexpr uint32_t TURBO_RESET_DELAY_MS = 500;
// Write handle 0x0012 is set directly after service discovery (hardcoded, confirmed from HCI log).
// CCCD is at NOTIFY_HANDLE + 1 (0x0016), confirmed from GATT attribute table.

// Forward declarations
class TrumaCoolerClimate;
class TrumaCoolerSwitch;

class TrumaCooler : public Component, public ble_client::BLEClientNode {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  // Sensor setters
  void set_temperature_sensor(sensor::Sensor *s) { temperature_sensor_ = s; }
  void set_ambient_temperature_sensor(sensor::Sensor *s) { ambient_temperature_sensor_ = s; }
  void set_compressor_running_sensor(binary_sensor::BinarySensor *s) { compressor_running_sensor_ = s; }
  void set_turbo_running_sensor(binary_sensor::BinarySensor *s) { turbo_running_sensor_ = s; }
  void set_device_on_sensor(binary_sensor::BinarySensor *s) { device_on_sensor_ = s; }
  void set_connected_sensor(binary_sensor::BinarySensor *s) { connected_sensor_ = s; }
  void set_climate(TrumaCoolerClimate *c) { climate_ = c; }
  void set_turbo_switch(TrumaCoolerSwitch *s) { turbo_switch_ = s; }

  void send_poll();
  void send_command(const uint8_t *cmd, size_t len);
  void set_mode(bool on);
  void set_setpoint(float temp_celsius);
  void set_turbo(bool state);

 protected:
  void parse_notification_(const uint8_t *data, uint16_t len);
  uint8_t calculate_checksum_(const uint8_t *data, size_t len);

  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  binary_sensor::BinarySensor *compressor_running_sensor_{nullptr};
  binary_sensor::BinarySensor *turbo_running_sensor_{nullptr};
  binary_sensor::BinarySensor *device_on_sensor_{nullptr};
  binary_sensor::BinarySensor *connected_sensor_{nullptr};
  TrumaCoolerClimate *climate_{nullptr};
  TrumaCoolerSwitch *turbo_switch_{nullptr};

  uint16_t write_handle_{0};
  bool connected_{false};
  uint32_t last_poll_{0};
  bool poll_enabled_{false};
  bool device_is_on_{false};
};

class TrumaCoolerClimate : public climate::Climate, public Parented<TrumaCooler> {
 public:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;
};

class TrumaCoolerSwitch : public switch_::Switch, public Parented<TrumaCooler> {
 protected:
  void write_state(bool state) override;
};

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

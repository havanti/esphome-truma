#include "truma_cooler_c44.h"

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace truma_cooler {

static const char *const TAG = "truma_cooler.c44";

// Turbo ON (device must already be ON): byte[4]=0x01, byte[9]=0x03
static constexpr uint8_t CMD_TURBO[FRAME_LEN] = {
    0xAA, 0xC1, 0xF1, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61
};
// Turbo OFF (device must already be ON): byte[4]=0x00, byte[9]=0x03
static constexpr uint8_t CMD_TURBO_OFF[FRAME_LEN] = {
    0xAA, 0xC1, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60
};

void TrumaCoolerC44::handle_status_(const uint8_t *data) {
  bool device_on = (data[4] == C44_DEVICE_ON);
  device_is_on_.store(device_on);
  bool compressor_running = (data[5] == C44_COMPRESSOR_RUNNING || data[5] == C44_COMPRESSOR_TURBO);
  bool turbo_running = (data[5] == C44_COMPRESSOR_TURBO);
  float interior_temp = (int8_t) data[6];  // interior temperature in °C (signed)
  int8_t setpoint = (int8_t) data[7];      // setpoint echoed back from device
  // Signed: HCI snoops only showed positive ambient temps, but int8_t extends
  // the range to -12.8..+12.7°C without breaking observed values.
  float ambient_temp = (int8_t) data[11] * 0.1f;

  ESP_LOGD(TAG, "Status: device=%s compressor=%s turbo=%s (byte5=0x%02X) interior=%.0f°C ambient=%.1f°C setpoint=%d°C",
           device_on ? "ON" : "OFF",
           compressor_running ? "RUNNING" : "OFF",
           turbo_running ? "ON" : "OFF",
           data[5], interior_temp, ambient_temp, setpoint);

  // Defer entity mutations to the main loop — gattc notifications run on the BT task.
  this->defer([this, device_on, compressor_running, turbo_running, interior_temp, ambient_temp, setpoint]() {
    if (temperature_sensor_ != nullptr)
      temperature_sensor_->publish_state(interior_temp);
    if (ambient_temperature_sensor_ != nullptr)
      ambient_temperature_sensor_->publish_state(ambient_temp);
    if (compressor_running_sensor_ != nullptr)
      compressor_running_sensor_->publish_state(compressor_running);
    if (turbo_running_sensor_ != nullptr)
      turbo_running_sensor_->publish_state(turbo_running);
    if (device_on_sensor_ != nullptr)
      device_on_sensor_->publish_state(device_on);

    if (climate_ != nullptr) {
      climate_->current_temperature = interior_temp;
      climate_->target_temperature = (float) setpoint;
      climate_->mode = device_on ? climate::CLIMATE_MODE_COOL : climate::CLIMATE_MODE_OFF;
      climate_->action = !device_on          ? climate::CLIMATE_ACTION_OFF
                         : compressor_running ? climate::CLIMATE_ACTION_COOLING
                                              : climate::CLIMATE_ACTION_IDLE;
      climate_->publish_state();
    }
  });
}

void TrumaCoolerC44::reset_entities_() {
  if (temperature_sensor_ != nullptr) temperature_sensor_->publish_state(NAN);
  if (ambient_temperature_sensor_ != nullptr) ambient_temperature_sensor_->publish_state(NAN);
  if (turbo_running_sensor_ != nullptr) turbo_running_sensor_->publish_state(false);
  if (climate_ != nullptr) {
    climate_->mode = climate::CLIMATE_MODE_OFF;
    climate_->action = climate::CLIMATE_ACTION_OFF;
    climate_->current_temperature = NAN;
    climate_->publish_state();
  }
}

void TrumaCoolerC44::apply_restored_states_() {
  if (climate_ != nullptr) climate_->apply_restored_state();
}

void TrumaCoolerC44::set_zone_setpoint(uint8_t zone, float temp_celsius) {
  (void) zone;  // single zone
  if (temp_celsius < SETPOINT_MIN_C) temp_celsius = SETPOINT_MIN_C;
  if (temp_celsius > SETPOINT_MAX_C) temp_celsius = SETPOINT_MAX_C;
  int8_t sp = (int8_t) lroundf(temp_celsius);
  ESP_LOGI(TAG, "setpoint: %d°C", sp);
  send_setpoint_frame_((uint8_t) sp, 0x00, ZONE1_SELECT);
}

void TrumaCoolerC44::set_turbo(bool state) {
  // Protocol requires device to be ON before turbo commands take effect.
  if (!device_is_on_.load()) {
    ESP_LOGW(TAG, "Turbo command ignored — device is OFF");
    if (turbo_switch_ != nullptr) turbo_switch_->publish_state(false);
    return;
  }
  if (state) {
    ESP_LOGI(TAG, "Turbo: ON");
    send_command(CMD_TURBO, sizeof(CMD_TURBO));
  } else {
    ESP_LOGI(TAG, "Turbo: OFF");
    send_command(CMD_TURBO_OFF, sizeof(CMD_TURBO_OFF));
  }
}

void TrumaCoolerC44::post_power_(bool on) {
  if (on) {
    // Always reset turbo shortly after ON so it starts in normal mode regardless
    // of the previously stored turbo state on the device.
    this->set_timeout("turbo_reset", C44_TURBO_RESET_DELAY_MS, [this]() {
      ESP_LOGI(TAG, "Turbo reset after ON");
      send_command(CMD_TURBO_OFF, sizeof(CMD_TURBO_OFF));
      if (turbo_switch_ != nullptr) turbo_switch_->publish_state(false);
    });
  } else {
    // Cancel any pending turbo reset from a recent ON — otherwise a queued
    // CMD_TURBO_OFF would be sent to an already-off device.
    this->cancel_timeout("turbo_reset");
    if (turbo_switch_ != nullptr) turbo_switch_->publish_state(false);
  }
}

void TrumaCoolerC44::on_disconnect_cleanup_() {
  this->cancel_timeout("turbo_reset");
  if (turbo_switch_ != nullptr) turbo_switch_->publish_state(false);
}

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

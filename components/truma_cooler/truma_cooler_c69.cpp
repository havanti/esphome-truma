#include "truma_cooler_c69.h"

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace truma_cooler {

static const char *const TAG = "truma_cooler.c69";

void TrumaCoolerC69::handle_status_(const uint8_t *data) {
  bool device_on = (data[4] & C69_DEVICE_ON_MASK) != 0;
  device_is_on_.store(device_on);
  // Compressor idle bit is only meaningful while powered on.
  bool compressor_running = device_on && (data[5] & C69_COMPRESSOR_IDLE_MASK) == 0;

  float zone1_interior = (int8_t) data[6];
  int8_t zone1_setpoint = (int8_t) data[7];
  float zone2_interior = (int8_t) data[8];
  int8_t zone2_setpoint = (int8_t) data[9];

  ESP_LOGD(TAG, "Status: device=%s compressor=%s (byte5=0x%02X) "
                "z1: interior=%.0f°C setpoint=%d°C | z2: interior=%.0f°C setpoint=%d°C",
           device_on ? "ON" : "OFF", compressor_running ? "RUNNING" : "OFF", data[5],
           zone1_interior, zone1_setpoint, zone2_interior, zone2_setpoint);

  // Defer entity mutations to the main loop — gattc notifications run on the BT task.
  this->defer([this, device_on, compressor_running, zone1_interior, zone1_setpoint,
               zone2_interior, zone2_setpoint]() {
    if (device_on_sensor_ != nullptr)
      device_on_sensor_->publish_state(device_on);
    if (power_switch_ != nullptr)
      power_switch_->publish_state(device_on);
    if (compressor_running_sensor_ != nullptr)
      compressor_running_sensor_->publish_state(compressor_running);
    if (temperature_zone1_sensor_ != nullptr)
      temperature_zone1_sensor_->publish_state(zone1_interior);
    if (temperature_zone2_sensor_ != nullptr)
      temperature_zone2_sensor_->publish_state(zone2_interior);
    publish_zone_(climate_zone1_, zone1_interior, zone1_setpoint, device_on, compressor_running);
    publish_zone_(climate_zone2_, zone2_interior, zone2_setpoint, device_on, compressor_running);
  });
}

void TrumaCoolerC69::publish_zone_(TrumaCoolerClimate *c, float interior, int8_t setpoint,
                                   bool device_on, bool compressor_running) {
  if (c == nullptr) return;
  c->current_temperature = interior;
  c->target_temperature = (float) setpoint;
  // COOL-only zones: on/off is shown via action + the master power switch.
  c->mode = climate::CLIMATE_MODE_COOL;
  c->action = !device_on          ? climate::CLIMATE_ACTION_OFF
              : compressor_running ? climate::CLIMATE_ACTION_COOLING
                                   : climate::CLIMATE_ACTION_IDLE;
  c->publish_state();
}

void TrumaCoolerC69::reset_entities_() {
  if (temperature_zone1_sensor_ != nullptr) temperature_zone1_sensor_->publish_state(NAN);
  if (temperature_zone2_sensor_ != nullptr) temperature_zone2_sensor_->publish_state(NAN);
  for (auto *c : {climate_zone1_, climate_zone2_}) {
    if (c == nullptr) continue;
    c->mode = climate::CLIMATE_MODE_COOL;  // COOL-only; off is expressed via action
    c->action = climate::CLIMATE_ACTION_OFF;
    c->current_temperature = NAN;
    c->publish_state();
  }
}

void TrumaCoolerC69::apply_restored_states_() {
  if (climate_zone1_ != nullptr) climate_zone1_->apply_restored_state();
  if (climate_zone2_ != nullptr) climate_zone2_->apply_restored_state();
}

void TrumaCoolerC69::set_zone_setpoint(uint8_t zone, float temp_celsius) {
  if (temp_celsius < SETPOINT_MIN_C) temp_celsius = SETPOINT_MIN_C;
  if (temp_celsius > SETPOINT_MAX_C) temp_celsius = SETPOINT_MAX_C;
  int8_t sp = (int8_t) lroundf(temp_celsius);
  if (zone == 2) {
    ESP_LOGI(TAG, "zone 2 setpoint: %d°C", sp);
    send_setpoint_frame_(0x00, (uint8_t) sp, ZONE2_SELECT);
  } else {
    ESP_LOGI(TAG, "zone 1 setpoint: %d°C", sp);
    send_setpoint_frame_((uint8_t) sp, 0x00, ZONE1_SELECT);
  }
}

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

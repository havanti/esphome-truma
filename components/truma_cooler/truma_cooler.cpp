#include "truma_cooler.h"

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "esphome/core/log.h"
#include <cmath>
#include <cstring>

namespace esphome {
namespace truma_cooler {

static const char *const TAG = "truma_cooler";

// Poll command
static const uint8_t CMD_POLL[] = {
    0xAA, 0xC1, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5C
};
// Turn ON:  byte[3]=0x01 — confirmed from HCI log: every OFF→ON transition uses this exact frame
static const uint8_t CMD_ON[] = {
    0xAA, 0xC1, 0xF1, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5E
};
// Turn OFF: byte[9]=0x00 — confirmed from HCI log (NOTIF OFF follows immediately)
static const uint8_t CMD_OFF[] = {
    0xAA, 0xC1, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D
};
// Turbo ON (device must already be ON): byte[4]=0x01, byte[9]=0x03
static const uint8_t CMD_TURBO[] = {
    0xAA, 0xC1, 0xF1, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61
};
// Turbo OFF (device must already be ON): byte[4]=0x00, byte[9]=0x03
static const uint8_t CMD_TURBO_OFF[] = {
    0xAA, 0xC1, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60
};

// ---------------------------------------------------------------------------
// TrumaCooler
// ---------------------------------------------------------------------------

void TrumaCooler::setup() {
  ESP_LOGCONFIG(TAG, "TrumaCooler component setup");
  // HCI snoop analysis shows the TrumaCooler does not require encryption or bonding.
  // Notifications and commands work without authentication.
}

void TrumaCooler::loop() {
  if (!connected_ || write_handle_ == 0) return;

  // Poll as fallback — device sends unsolicited notifications every ~2s.
  if (poll_enabled_ && millis() - last_poll_ > POLL_INTERVAL_MS) {
    send_poll();
    last_poll_ = millis();
  }
}

void TrumaCooler::dump_config() {
  ESP_LOGCONFIG(TAG, "TrumaCooler BLE:");
  ESP_LOGCONFIG(TAG, "  MAC: %s", this->parent_->address_str());
  ESP_LOGCONFIG(TAG, "  Service UUID: 0xFFF0");
  ESP_LOGCONFIG(TAG, "  Write Handle: 0x%04X", write_handle_);
  ESP_LOGCONFIG(TAG, "  Notify Handle: 0x%04X", NOTIFY_HANDLE);
}

void TrumaCooler::gattc_event_handler(esp_gattc_cb_event_t event,
                                       esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "Connected to TrumaCooler");
        connected_ = true;
        if (connected_sensor_ != nullptr) connected_sensor_->publish_state(true);
        // Reset sensors to unknown until first notification arrives
        if (temperature_sensor_ != nullptr) temperature_sensor_->publish_state(NAN);
        if (compressor_running_sensor_ != nullptr) compressor_running_sensor_->publish_state(false);
      }
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGI(TAG, "Disconnected from TrumaCooler");
      connected_ = false;
      poll_enabled_ = false;
      write_handle_ = 0;
      device_is_on_ = false;
      if (connected_sensor_ != nullptr) connected_sensor_->publish_state(false);
      if (temperature_sensor_ != nullptr) temperature_sensor_->publish_state(NAN);
      if (ambient_temperature_sensor_ != nullptr) ambient_temperature_sensor_->publish_state(NAN);
      if (compressor_running_sensor_ != nullptr) compressor_running_sensor_->publish_state(false);
      if (turbo_running_sensor_ != nullptr) turbo_running_sensor_->publish_state(false);
      if (device_on_sensor_ != nullptr) device_on_sensor_->publish_state(false);
      if (turbo_switch_ != nullptr) turbo_switch_->publish_state(false);
      if (climate_ != nullptr) {
        climate_->mode = climate::CLIMATE_MODE_OFF;
        climate_->action = climate::CLIMATE_ACTION_OFF;
        climate_->publish_state();
      }
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      write_handle_ = 0x0012;
      poll_enabled_ = false;
      ESP_LOGI(TAG, "Service discovery done. Registering for notify (no encryption required).");

      auto reg_ret = esp_ble_gattc_register_for_notify(gattc_if,
                                                        this->parent_->get_remote_bda(),
                                                        NOTIFY_HANDLE);
      ESP_LOGI(TAG, "register_for_notify handle=0x%04X ret=%d", NOTIFY_HANDLE, reg_ret);

      // HCI snoop analysis: cooler uses unauthenticated writes — no bonding needed.
      uint8_t notify_en[] = {0x01, 0x00};
      auto cccd_ret = esp_ble_gattc_write_char_descr(gattc_if,
                                                      this->parent_->get_conn_id(),
                                                      NOTIFY_HANDLE + 1, sizeof(notify_en),
                                                      notify_en, ESP_GATT_WRITE_TYPE_RSP,
                                                      ESP_GATT_AUTH_REQ_NONE);
      ESP_LOGI(TAG, "CCCD write ret=%d", cccd_ret);
      poll_enabled_ = true;
      last_poll_ = millis();
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
      ESP_LOGI(TAG, "register_for_notify complete: status=%d handle=0x%04X",
               param->reg_for_notify.status, param->reg_for_notify.handle);
      break;

    case ESP_GATTC_WRITE_DESCR_EVT:
      ESP_LOGI(TAG, "CCCD write result: status=%d handle=0x%04X",
               param->write.status, param->write.handle);
      break;

    case ESP_GATTC_NOTIFY_EVT:
      if (param->notify.handle == NOTIFY_HANDLE) {
        parse_notification_(param->notify.value, param->notify.value_len);
      }
      break;

    default:
      break;
  }
}

void TrumaCooler::parse_notification_(const uint8_t *data, uint16_t len) {
  if (len < FRAME_LEN) {
    ESP_LOGW(TAG, "Notification too short: %d bytes", len);
    return;
  }

  if (data[0] != HEADER_B0 || data[1] != HEADER_B1 || data[2] != RESPONSE_TYPE) {
    ESP_LOGW(TAG, "Invalid header: %02X %02X %02X", data[0], data[1], data[2]);
    return;
  }

  uint8_t expected = calculate_checksum_(data, FRAME_LEN - 1);
  if (data[FRAME_LEN - 1] != expected) {
    ESP_LOGW(TAG, "Checksum mismatch: expected 0x%02X got 0x%02X", expected, data[FRAME_LEN - 1]);
    return;
  }

  bool device_on = (data[4] == DEVICE_ON);
  device_is_on_ = device_on;
  bool compressor_running = (data[5] == COMPRESSOR_RUNNING || data[5] == COMPRESSOR_TURBO);
  bool turbo_running = (data[5] == COMPRESSOR_TURBO);
  float interior_temp = (int8_t)data[6];        // interior temperature in °C (signed)
  int8_t setpoint = (int8_t)data[7];             // setpoint echoed back from device
  // Signed: HCI snoops only showed positive ambient temps, but int8_t extends
  // the range to -12.8..+12.7°C without breaking observed values.
  float ambient_temp = (int8_t)data[11] * 0.1f;

  ESP_LOGD(TAG, "Status: device=%s compressor=%s turbo=%s (byte5=0x%02X) interior=%.0f°C ambient=%.1f°C setpoint=%d°C",
           device_on ? "ON" : "OFF",
           compressor_running ? "RUNNING" : "OFF",
           turbo_running ? "ON" : "OFF",
           data[5], interior_temp, ambient_temp, setpoint);

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
    climate_->target_temperature = (float)setpoint;
    climate_->mode = device_on ? climate::CLIMATE_MODE_COOL : climate::CLIMATE_MODE_OFF;
    climate_->action = !device_on          ? climate::CLIMATE_ACTION_OFF
                       : compressor_running ? climate::CLIMATE_ACTION_COOLING
                                           : climate::CLIMATE_ACTION_IDLE;
    climate_->publish_state();
  }
}

uint8_t TrumaCooler::calculate_checksum_(const uint8_t *data, size_t len) {
  uint16_t sum = 0;
  for (size_t i = 0; i < len; i++) sum += data[i];
  return (uint8_t)((sum + 1) & 0xFF);
}

void TrumaCooler::send_poll() { send_command(CMD_POLL, sizeof(CMD_POLL)); }

void TrumaCooler::set_mode(bool on) {
  device_is_on_ = on;
  if (on) {
    // Send fixed ON command — device uses its internally stored setpoint.
    // Do NOT send a setpoint command before this: it triggers temperature-input
    // mode on the display and the ON command is ignored.
    ESP_LOGI(TAG, "control: ON");
    send_command(CMD_ON, sizeof(CMD_ON));
    // Always reset turbo shortly after ON so it starts in normal mode regardless
    // of the previously stored turbo state on the device.
    this->set_timeout("turbo_reset", TURBO_RESET_DELAY_MS, [this]() {
      ESP_LOGI(TAG, "Turbo reset after ON");
      send_command(CMD_TURBO_OFF, sizeof(CMD_TURBO_OFF));
      if (turbo_switch_ != nullptr)
        turbo_switch_->publish_state(false);
    });
  } else {
    // Fixed OFF command (byte[4]=0x00, byte[9]=0x00)
    ESP_LOGI(TAG, "control: OFF");
    send_command(CMD_OFF, sizeof(CMD_OFF));
    if (turbo_switch_ != nullptr)
      turbo_switch_->publish_state(false);
  }
}

void TrumaCooler::set_turbo(bool state) {
  if (state) {
    ESP_LOGI(TAG, "Turbo: ON");
    send_command(CMD_TURBO, sizeof(CMD_TURBO));
  } else {
    ESP_LOGI(TAG, "Turbo: OFF");
    send_command(CMD_TURBO_OFF, sizeof(CMD_TURBO_OFF));
  }
}

void TrumaCooler::set_setpoint(float temp_celsius) {
  if (temp_celsius < SETPOINT_MIN_C) temp_celsius = SETPOINT_MIN_C;
  if (temp_celsius > SETPOINT_MAX_C) temp_celsius = SETPOINT_MAX_C;
  int8_t sp = (int8_t)lroundf(temp_celsius);
  uint8_t cmd[FRAME_LEN] = {0xAA, 0xC1, 0xF1, 0x00, 0x00, 0x00, 0x00, (uint8_t)sp,
                            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  cmd[FRAME_LEN - 1] = calculate_checksum_(cmd, FRAME_LEN - 1);
  ESP_LOGI(TAG, "setpoint: %d°C", sp);
  send_command(cmd, sizeof(cmd));
}

void TrumaCooler::send_command(const uint8_t *cmd, size_t len) {
  if (!connected_ || write_handle_ == 0) {
    ESP_LOGW(TAG, "Cannot send: not connected");
    return;
  }

  ESP_LOGD(TAG, "Sending %02X %02X %02X... to handle 0x%04X", cmd[0], cmd[1], cmd[2], write_handle_);

  if (len > FRAME_LEN) {
    ESP_LOGW(TAG, "send_command: len %u > FRAME_LEN %u, truncated", (unsigned) len, (unsigned) FRAME_LEN);
    len = FRAME_LEN;
  }
  // IDF write_char takes uint8_t* (non-const) even for NO_RSP writes.
  uint8_t buf[FRAME_LEN];
  memcpy(buf, cmd, len);
  auto status = esp_ble_gattc_write_char(
      this->parent_->get_gattc_if(),
      this->parent_->get_conn_id(),
      write_handle_, len,
      buf,
      ESP_GATT_WRITE_TYPE_NO_RSP,
      ESP_GATT_AUTH_REQ_NONE);

  if (status != ESP_OK) {
    ESP_LOGW(TAG, "Write failed: 0x%X", status);
  } else {
    // Reset poll timer so next poll happens after full 60s interval.
    // The device sends unsolicited notifications every ~2s, so state
    // updates arrive automatically without needing an immediate poll.
    last_poll_ = millis();
  }
}

// ---------------------------------------------------------------------------
// TrumaCoolerClimate
// ---------------------------------------------------------------------------

climate::ClimateTraits TrumaCoolerClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_COOL});
  traits.set_visual_min_temperature(SETPOINT_MIN_C);
  traits.set_visual_max_temperature(SETPOINT_MAX_C);
  traits.set_visual_temperature_step(1);
  return traits;
}

void TrumaCoolerClimate::control(const climate::ClimateCall &call) {
  bool has_mode = call.get_mode().has_value();
  bool has_temp = call.get_target_temperature().has_value();

  if (has_mode) this->mode = *call.get_mode();
  if (has_temp) this->target_temperature = *call.get_target_temperature();

  if (has_mode) {
    bool on = (this->mode != climate::CLIMATE_MODE_OFF);
    this->parent_->set_mode(on);
    // Send setpoint after ON (never before — triggers display input mode and ON is ignored).
    if (on && has_temp)
      this->parent_->set_setpoint(this->target_temperature);
  } else if (has_temp) {
    this->parent_->set_setpoint(this->target_temperature);
  }

  this->publish_state();
}

// ---------------------------------------------------------------------------
// TrumaCoolerSwitch
// ---------------------------------------------------------------------------

void TrumaCoolerSwitch::write_state(bool state) {
  this->parent_->set_turbo(state);
  this->publish_state(state);
}

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

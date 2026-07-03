#include "truma_cooler.h"

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "esphome/core/log.h"
#include <cmath>
#include <cstring>

namespace esphome {
namespace truma_cooler {

static const char *const TAG = "truma_cooler";

// Shared command frames — identical across models, confirmed from HCI snoops
// of both the C44 and the C69 official app.
static constexpr uint8_t CMD_POLL[FRAME_LEN] = {
    0xAA, 0xC1, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5C
};
// Turn ON: byte[3]=0x01 — every OFF->ON transition uses this exact frame.
static constexpr uint8_t CMD_ON[FRAME_LEN] = {
    0xAA, 0xC1, 0xF1, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5E
};
// Turn OFF: all-zero payload.
static constexpr uint8_t CMD_OFF[FRAME_LEN] = {
    0xAA, 0xC1, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D
};

// ---------------------------------------------------------------------------
// TrumaCooler — shared plumbing
// ---------------------------------------------------------------------------

void TrumaCooler::setup() {
  ESP_LOGCONFIG(TAG, "TrumaCooler component setup (model %s)", model_name_());
  // HCI snoop analysis shows the cooler requires neither encryption nor bonding.
  apply_restored_states_();
}

void TrumaCooler::loop() {
  if (!connected_.load() || write_handle_.load() == 0) return;

  // Poll as fallback — device sends unsolicited notifications every ~2 s.
  if (poll_enabled_.load() && millis() - last_poll_ > POLL_INTERVAL_MS) {
    send_poll();
    last_poll_ = millis();
  }
}

void TrumaCooler::dump_config() {
  ESP_LOGCONFIG(TAG, "TrumaCooler BLE:");
  ESP_LOGCONFIG(TAG, "  Model: %s", model_name_());
  ESP_LOGCONFIG(TAG, "  MAC: %s", this->parent_->address_str());
  ESP_LOGCONFIG(TAG, "  Service UUID: 0xFFF0");
  ESP_LOGCONFIG(TAG, "  Write Handle: 0x%04X", write_handle_.load());
  ESP_LOGCONFIG(TAG, "  Notify Handle: 0x%04X", NOTIFY_HANDLE);
}

void TrumaCooler::gattc_event_handler(esp_gattc_cb_event_t event,
                                      esp_gatt_if_t gattc_if,
                                      esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "Connected to TrumaCooler");
        connected_.store(true);
        // gattc events fire on the BT controller task. Defer entity mutations
        // (publish_state, climate field writes, scheduler ops) to the main loop.
        this->defer([this]() {
          if (connected_sensor_ != nullptr) connected_sensor_->publish_state(true);
          if (device_on_sensor_ != nullptr) device_on_sensor_->publish_state(false);
          if (power_switch_ != nullptr) power_switch_->publish_state(false);
          if (compressor_running_sensor_ != nullptr) compressor_running_sensor_->publish_state(false);
          // Reset the model-specific entities to a known baseline.
          this->reset_entities_();
        });
      }
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGI(TAG, "Disconnected from TrumaCooler");
      connected_.store(false);
      poll_enabled_.store(false);
      write_handle_.store(0);
      device_is_on_.store(false);
      // Defer scheduler op + publishes to main loop (BT task is not the right context).
      this->defer([this]() {
        this->on_disconnect_cleanup_();
        if (connected_sensor_ != nullptr) connected_sensor_->publish_state(false);
        if (device_on_sensor_ != nullptr) device_on_sensor_->publish_state(false);
        if (power_switch_ != nullptr) power_switch_->publish_state(false);
        if (compressor_running_sensor_ != nullptr) compressor_running_sensor_->publish_state(false);
        this->reset_entities_();
      });
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      write_handle_.store(WRITE_HANDLE);
      poll_enabled_.store(false);
      ESP_LOGI(TAG, "Service discovery done. Registering for notify (no encryption required).");

      auto reg_ret = esp_ble_gattc_register_for_notify(gattc_if,
                                                       this->parent_->get_remote_bda(),
                                                       NOTIFY_HANDLE);
      ESP_LOGI(TAG, "register_for_notify handle=0x%04X ret=%d", NOTIFY_HANDLE, reg_ret);

      // HCI snoop analysis: cooler uses unauthenticated writes — no bonding needed.
      uint8_t notify_en[] = {0x01, 0x00};
      auto cccd_ret = esp_ble_gattc_write_char_descr(gattc_if,
                                                     this->parent_->get_conn_id(),
                                                     CCCD_HANDLE, sizeof(notify_en),
                                                     notify_en, ESP_GATT_WRITE_TYPE_RSP,
                                                     ESP_GATT_AUTH_REQ_NONE);
      ESP_LOGI(TAG, "CCCD write ret=%d", cccd_ret);
      poll_enabled_.store(true);
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

  handle_status_(data);
}

uint8_t TrumaCooler::calculate_checksum_(const uint8_t *data, size_t len) {
  uint16_t sum = 0;
  for (size_t i = 0; i < len; i++) sum += data[i];
  return (uint8_t)((sum + 1) & 0xFF);
}

void TrumaCooler::send_poll() { send_command(CMD_POLL, sizeof(CMD_POLL)); }

void TrumaCooler::set_mode(bool on) {
  device_is_on_.store(on);
  if (on) {
    // Send fixed ON command — device uses its internally stored setpoint.
    // Do NOT send a setpoint command before this: it triggers temperature-input
    // mode on the display and the ON command is ignored.
    ESP_LOGI(TAG, "control: ON");
    send_command(CMD_ON, sizeof(CMD_ON));
  } else {
    ESP_LOGI(TAG, "control: OFF");
    send_command(CMD_OFF, sizeof(CMD_OFF));
  }
  post_power_(on);
}

void TrumaCooler::send_setpoint_frame_(uint8_t byte7, uint8_t byte8, uint8_t zone_select) {
  uint8_t cmd[FRAME_LEN] = {0xAA, 0xC1, 0xF1, 0x00, 0x00, 0x00, 0x00, byte7,
                            byte8, zone_select, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  cmd[FRAME_LEN - 1] = calculate_checksum_(cmd, FRAME_LEN - 1);
  send_command(cmd, sizeof(cmd));
}

void TrumaCooler::send_command(const uint8_t *cmd, size_t len) {
  uint16_t handle = write_handle_.load();
  if (!connected_.load() || handle == 0) {
    ESP_LOGW(TAG, "Cannot send: not connected");
    return;
  }

  if (len > FRAME_LEN) {
    ESP_LOGW(TAG, "send_command: len %u > FRAME_LEN %u, truncated", (unsigned) len, (unsigned) FRAME_LEN);
    len = FRAME_LEN;
  }

  ESP_LOGD(TAG, "Sending [%s] to handle 0x%04X",
           format_hex_pretty(cmd, len).c_str(), handle);

  // IDF write_char takes uint8_t* (non-const) even for NO_RSP writes.
  uint8_t buf[FRAME_LEN];
  memcpy(buf, cmd, len);
  auto status = esp_ble_gattc_write_char(
      this->parent_->get_gattc_if(),
      this->parent_->get_conn_id(),
      handle, len,
      buf,
      ESP_GATT_WRITE_TYPE_NO_RSP,
      ESP_GATT_AUTH_REQ_NONE);

  if (status != ESP_OK) {
    ESP_LOGW(TAG, "Write failed: 0x%X", status);
  } else {
    // Reset poll timer so next poll happens after full POLL_INTERVAL_MS.
    // The device sends unsolicited notifications every ~2 s, so state
    // updates arrive automatically without needing an immediate poll.
    last_poll_ = millis();
  }
}

// ---------------------------------------------------------------------------
// TrumaCoolerClimate — zone-aware (zone 0 = single C44, 1/2 = C69 zones)
// ---------------------------------------------------------------------------

climate::ClimateTraits TrumaCoolerClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  if (manages_power_) {
    traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_COOL});
  } else {
    // Power lives on the master switch — expose COOL only, no OFF toggle here.
    traits.set_supported_modes({climate::CLIMATE_MODE_COOL});
  }
  traits.set_visual_min_temperature(SETPOINT_MIN_C);
  traits.set_visual_max_temperature(SETPOINT_MAX_C);
  traits.set_visual_temperature_step(1.0f);
  return traits;
}

void TrumaCoolerClimate::apply_restored_state() {
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
  }
  // COOL-only zones must never sit in OFF (not an advertised mode) — e.g. after a
  // restore persisted by an older firmware that still exposed the OFF mode.
  if (!manages_power_ && this->mode == climate::CLIMATE_MODE_OFF)
    this->mode = climate::CLIMATE_MODE_COOL;
}

void TrumaCoolerClimate::control(const climate::ClimateCall &call) {
  bool has_temp = call.get_target_temperature().has_value();
  if (has_temp) this->target_temperature = *call.get_target_temperature();

  if (!manages_power_) {
    // Master switch owns power; this COOL-only climate just writes the setpoint.
    if (has_temp)
      this->parent_->set_zone_setpoint(this->zone_, this->target_temperature);
    this->publish_state();
    return;
  }

  bool has_mode = call.get_mode().has_value();
  if (has_mode) this->mode = *call.get_mode();

  if (has_mode) {
    // Power is global on both models — turning any zone off powers the whole box.
    bool on = (this->mode != climate::CLIMATE_MODE_OFF);
    this->parent_->set_mode(on);
    // Send setpoint after ON (never before — triggers display input mode and ON is ignored).
    if (on && has_temp)
      this->parent_->set_zone_setpoint(this->zone_, this->target_temperature);
  } else if (has_temp) {
    this->parent_->set_zone_setpoint(this->zone_, this->target_temperature);
  }

  this->publish_state();
}

// ---------------------------------------------------------------------------
// TrumaCoolerSwitch (turbo — C44 only)
// ---------------------------------------------------------------------------

void TrumaCoolerSwitch::write_state(bool state) {
  this->parent_->set_turbo(state);
  this->publish_state(state);
}

// ---------------------------------------------------------------------------
// TrumaCoolerPowerSwitch (master power — C69)
// ---------------------------------------------------------------------------

void TrumaCoolerPowerSwitch::write_state(bool state) {
  // Optimistic publish; the next status notification confirms the real state.
  this->parent_->set_mode(state);
  this->publish_state(state);
}

}  // namespace truma_cooler
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

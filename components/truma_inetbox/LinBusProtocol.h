#pragma once

#include <array>
#include "LinBusListener.h"

namespace esphome {
namespace truma_inetbox {

// Largest answer is a 41 byte StatusFrame = 7 LIN TP frames. Leaves room for pending single frame answers.
static constexpr UBaseType_t LIN_UPDATE_QUEUE_LENGTH = 16;

class LinBusProtocol : public LinBusListener {
 public:
  virtual const std::array<uint8_t, 4> lin_identifier() = 0;
  virtual void lin_heartbeat() = 0;
  virtual void lin_reset_device();

 protected:
  const std::array<uint8_t, 8> lin_empty_response_ = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  bool answer_lin_order_(const uint8_t pid) override;
  void lin_message_received_(const uint8_t pid, const uint8_t *message, uint8_t length) override;

  // Data bytes of a Read-by-Identifier answer (up to 5) and their number; 0 = identifier not supported.
  virtual uint8_t lin_read_field_by_identifier_(uint8_t identifier, std::array<uint8_t, 5> *response) = 0;
  virtual const uint8_t *lin_multiframe_received(const uint8_t *message, const uint8_t message_len,
                                                  uint8_t *return_len) = 0;

  bool has_update_to_send_() { return uxQueueMessagesWaiting(this->updates_to_send_) > 0; }

 private:
  uint8_t lin_node_address_ = /*LIN initial node address*/ 0x03;

  // Filled by lin_event_task, drained by uartEventTask_ on PID 0x3D. A FreeRTOS queue because both tasks
  // run on the same core and the UART task preempts the LIN task (std::queue is not safe for this).
  // Declaration order matters: storage → static-queue-struct → handle.
  uint8_t updates_to_send_storage_[LIN_UPDATE_QUEUE_LENGTH * sizeof(std::array<uint8_t, 8>)];
  StaticQueue_t updates_to_send_static_;
  QueueHandle_t updates_to_send_ =
      xQueueCreateStatic(LIN_UPDATE_QUEUE_LENGTH, sizeof(std::array<uint8_t, 8>), updates_to_send_storage_,
                         &updates_to_send_static_);

  void prepare_update_msg_(const std::array<uint8_t, 8> &message);
  bool is_matching_identifier_(const uint8_t *message);

  uint16_t multi_pdu_message_expected_size_ = 0;
  uint8_t multi_pdu_message_len_ = 0;
  uint8_t multi_pdu_message_frame_counter_ = 0;
  uint8_t multi_pdu_message_[64];
  void lin_msg_diag_single_(const uint8_t *message, uint8_t length);
  void lin_msg_diag_first_(const uint8_t *message, uint8_t length);
  bool lin_msg_diag_consecutive_(const uint8_t *message, uint8_t length);
  void lin_msg_diag_multi_();
};

}  // namespace truma_inetbox
}  // namespace esphome
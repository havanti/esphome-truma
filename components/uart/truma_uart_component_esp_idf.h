#pragma once

#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "uart_component_esp_idf.h"

namespace esphome {
namespace uart {

class truma_IDFUARTComponent : public IDFUARTComponent {
 public:
  // `uart_event_queue_` is always available in IDFUARTComponent (not conditional).
  QueueHandle_t *get_uart_event_queue() { return &this->uart_event_queue_; }
};

}  // namespace uart
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF

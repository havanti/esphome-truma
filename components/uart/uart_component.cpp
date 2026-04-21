#include "uart_component.h"

namespace esphome {
namespace uart {

static const char *const TAG = "uart";

bool UARTComponent::check_read_timeout_(size_t len) {
  if (this->available() >= len)
    return true;

  uint32_t start_time = millis();
  while (this->available() < len) {
    if (millis() - start_time > 100) {
      ESP_LOGE(TAG, "Reading from UART timed out at byte %u!", (unsigned) this->available());
      return false;
    }
    yield();
  }
  return true;
}

}  // namespace uart
}  // namespace esphome

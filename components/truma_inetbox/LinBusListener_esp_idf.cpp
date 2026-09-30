#ifdef USE_ESP32_FRAMEWORK_ESP_IDF
#include "LinBusListener.h"
#include "esphome/core/log.h"
#include "soc/uart_reg.h"
#ifdef CUSTOM_ESPHOME_UART
#include "esphome/components/uart/truma_uart_component_esp_idf.h"
#endif // CUSTOM_ESPHOME_UART
#include "esphome/components/uart/uart_component_esp_idf.h"

namespace esphome {
namespace truma_inetbox {

#ifdef CUSTOM_ESPHOME_UART
using EsphomeUart = uart::truma_IDFUARTComponent;
#else
using EsphomeUart = uart::IDFUARTComponent;
#endif  // CUSTOM_ESPHOME_UART

static const char *const TAG = "truma_inetbox.LinBusListener";

// Priority 24: above Wi-Fi (23) so LIN BREAK detection is not delayed by network traffic.
static constexpr UBaseType_t UART_EVENT_TASK_PRIORITY = 24;
// Priority 2: low — LIN message processing runs after time-critical UART task.
static constexpr UBaseType_t LIN_EVENT_TASK_PRIORITY = 2;

static constexpr TickType_t QUEUE_WAIT_BLOCKING = portMAX_DELAY;

// Default stack size of 2048 is not enough for the LIN processing.
static constexpr uint32_t LIN_TASK_STACK_SIZE = 4096;
// Run the UART and LIN tasks on core 0, ESPHome runs on core 1.
static constexpr BaseType_t LIN_TASK_CORE = 0;

void LinBusListener::setup_framework() {
  // uartSetFastReading
  auto uartComp = static_cast<EsphomeUart *>(this->parent_);

  auto uart_num = uartComp->get_hw_serial_number();

  // Tweak the fifo settings so data is available as soon as the first byte is received.
  // If not it will wait either until fifo is filled or a certain time has passed.
  uart_intr_config_t uart_intr{};
  uart_intr.intr_enable_mask =
      UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M;  // only these IRQs - no BREAK, PARITY or OVERFLOW
  uart_intr.rxfifo_full_thresh =
      1;  // 1 byte threshold (default 120 would delay first byte significantly)
  uart_intr.rx_timeout_thresh =
      10;  // 10 tick timeout (sufficient for short LIN frames)
  uart_intr.txfifo_empty_intr_thresh = 10;  // UART_EMPTY_THRESH_DEFAULT
  esp_err_t err = uart_intr_config((uart_port_t) uart_num, &uart_intr);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, " -- UART%d interrupt config failed: %s", uart_num, esp_err_to_name(err));
  }

  // Creating UART event Task
  xTaskCreatePinnedToCore(LinBusListener::uartEventTask_,
                          "uart_event_task",                      // name
                          LIN_TASK_STACK_SIZE,                    // stack size (in words)
                          this,                                   // input params
                          UART_EVENT_TASK_PRIORITY,               // priority
                          &this->uartEventTaskHandle_,            // handle
                          LIN_TASK_CORE                           // core
  );
  if (this->uartEventTaskHandle_ == NULL) {
    ESP_LOGE(TAG, " -- UART%d Event Task not created!", uart_num);
  }

  // Creating LIN msg event Task
  xTaskCreatePinnedToCore(LinBusListener::eventTask_,
                          "lin_event_task",                       // name
                          LIN_TASK_STACK_SIZE,                    // stack size (in words)
                          this,                                   // input params
                          LIN_EVENT_TASK_PRIORITY,                // priority
                          &this->eventTaskHandle_,                // handle
                          LIN_TASK_CORE                           // core
  );

  if (this->eventTaskHandle_ == NULL) {
    ESP_LOGE(TAG, " -- LIN message Task not created!");
  }
}

void LinBusListener::uartEventTask_(void *args) {
  LinBusListener *instance = (LinBusListener *) args;
  auto uartComp = static_cast<EsphomeUart *>(instance->parent_);
  auto uart_num = uartComp->get_hw_serial_number();
  auto uartEventQueue = uartComp->get_uart_event_queue();
  // Wait for UART event queue to be initialized.
  // On dual-core ESP32 the task may start on core 0 before core 1 has finished
  // calling uart_driver_install(), leaving the queue handle momentarily NULL.
  uint32_t waited_ms = 0;
  while (*uartEventQueue == nullptr) {
    vTaskDelay(pdMS_TO_TICKS(1));
    if (++waited_ms > 5000) {
      ESP_LOGE(TAG, "UART%d event queue not available after 5s -- UART setup failed?", uart_num);
      vTaskDelete(NULL);
      return;
    }
  }
  uart_event_t event;
  for (;;) {
    // Waiting for UART event.
    if (xQueueReceive(*uartEventQueue, (void *) &event, QUEUE_WAIT_BLOCKING)) {
      if (event.type == UART_DATA && instance->available() > 0) {
        instance->onReceive_();
      } else if (event.type == UART_BREAK) {
        // Drain any bytes still in the FIFO before processing the BREAK.
        // The CRC byte of the current frame may arrive in the FIFO at the same time as
        // the BREAK event is queued; without draining, it gets misread as the BREAK byte
        // of the next frame and the frame appears incomplete (partial data warning).
        if (instance->available() > 0) {
          instance->onReceive_();
        }
        // If break is valid, onReceive is called first and handled — state should be waiting for SYNC.
        if (instance->current_state_ != READ_STATE_SYNC) {
          instance->current_state_ = READ_STATE_BREAK;
        }
      }
    }
  }
  vTaskDelete(NULL);
}

void LinBusListener::eventTask_(void *args) {
  LinBusListener *instance = (LinBusListener *) args;
  for (;;) {
    instance->process_lin_msg_queue(QUEUE_WAIT_BLOCKING);
  }
}

}  // namespace truma_inetbox
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF
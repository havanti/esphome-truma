#include "TrumaVersionTextSensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.text_sensor";

void TrumaVersionTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "Truma Version Text Sensor", this);
}

}  // namespace truma_inetbox
}  // namespace esphome

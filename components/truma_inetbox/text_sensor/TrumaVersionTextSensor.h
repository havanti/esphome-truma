#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/truma_inetbox/version.h"

namespace esphome {
namespace truma_inetbox {

class TrumaVersionTextSensor : public Component, public text_sensor::TextSensor {
 public:
  void setup() override { this->publish_state(TRUMA_INETBOX_VERSION); }
  void dump_config() override;
};

}  // namespace truma_inetbox
}  // namespace esphome

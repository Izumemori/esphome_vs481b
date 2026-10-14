#pragma once

#include "../vs481b.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace vs481b {

class FirmwareSensor : public Component, public text_sensor::TextSensor, public Parented<VS481B> {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace vs481b
}  // namespace esphome

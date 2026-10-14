#pragma once

#include "../vs481b.h"
#include "esphome/components/select/select.h"

namespace esphome {
namespace vs481b {

class AutoPortSelect : public Component, public select::Select, public Parented<VS481B> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;
};

}  // namespace vs481b
}  // namespace esphome

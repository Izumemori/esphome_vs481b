#pragma once

#include "../vs481b.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace vs481b {

class HPDSwitch : public Component, public switch_::Switch, public Parented<VS481B> {
 public:
  void setup() override;
  void dump_config() override;

  // The switch doesn't report HPD in its read reply, so this is only the last value sent
  bool assumed_state() override { return true; }

 protected:
  void write_state(bool state) override;
};

}  // namespace vs481b
}  // namespace esphome

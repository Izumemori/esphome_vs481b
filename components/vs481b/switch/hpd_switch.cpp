#include "hpd_switch.h"

namespace esphome {
namespace vs481b {
static const char *const TAG = "vs481b.hpd_switch";

void HPDSwitch::setup() {
  this->parent_->hpd_callback.add([this](bool state) { this->publish_state(state); });
}

void HPDSwitch::dump_config() { LOG_SWITCH("", "VS481B HPD Switch", this); }

void HPDSwitch::write_state(bool state) {
  // State is published by hpd_callback once the switch answers
  this->parent_->set_hpd(state);
}
}  // namespace vs481b
}  // namespace esphome

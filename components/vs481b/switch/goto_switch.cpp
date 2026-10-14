#include "goto_switch.h"

namespace esphome{
namespace vs481b {
    static const char *const TAG = "vs481b.goto_switch";

    void GotoSwitch::setup() {
        this->parent_->goto_callback.add([this](bool state) {
            this->publish_state(state);
        });
    }

    void GotoSwitch::dump_config() {
        LOG_SWITCH("", "VS481B Goto Switch", this);
    }

    void GotoSwitch::write_state(bool state) {
        // State is published by goto_callback once the switch answers
        this->parent_->set_goto(state);
    }
}
}

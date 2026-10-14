#include "output_switch.h"

namespace esphome{
namespace vs481b {
    static const char *const TAG = "vs481b.output_switch";

    void OutputSwitch::setup() {
        this->parent_->output_callback.add([this](bool state) {
            this->publish_state(state);
        });
    }

    void OutputSwitch::dump_config() {
        LOG_SWITCH("", "VS481B Output Switch", this);
    }

    void OutputSwitch::write_state(bool state) {
        // State is published by output_callback once the switch answers
        this->parent_->set_output(state);
    }
}
}

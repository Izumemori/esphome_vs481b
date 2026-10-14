#include "output_switch.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.output_switch";

    void OutputSwitch::setup() {
        this->parent->output_callback.add([this](bool state) {
            this->publish_state(state);
        }) ;
    }

    void OutputSwitch::write_state(bool state) {
        // State is published by output_callback once the switch answers
        this->parent->set_output(state);
    }
}
}
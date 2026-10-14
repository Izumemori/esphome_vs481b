#include "goto_switch.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.goto_switch";

    void GotoSwitch::setup() {
        this->parent->goto_callback.add([this](bool state) {
            this->publish_state(state);
        }) ;
    }

    void GotoSwitch::write_state(bool state) {
        // State is published by goto_callback once the switch answers
        this->parent->set_goto(state);
    }
}
}
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
        if (!this->parent->set_goto(state))
            this->publish_state(this->state);
    }
}
}
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
        if (!this->parent->set_output(state))
            this->publish_state(this->state);
    }
}
}
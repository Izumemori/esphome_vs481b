#include "mode_select.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.mode_select";

    void ModeSelect::setup() {
        // Options are set from codegen
        this->parent->mode_callback.add([this](Mode state){
            this->publish_state(mode_to_str(state));
        });
    }

    void ModeSelect::control(size_t index) {
        // State is published by mode_callback once the switch answers
        this->parent->set_mode(str_to_mode(this->option_at(index)));
    }
}
}

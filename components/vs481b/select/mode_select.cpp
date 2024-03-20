#include "mode_select.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.mode_select";

    void ModeSelect::setup() {
        this->parent->mode_callback.add([this](Mode state){
            this->publish_state(mode_to_str(state));
        });

        std::vector<std::string> modes = {
            mode_to_str(Mode::M_NEXT),
            mode_to_str(Mode::M_DEFAULT),
            mode_to_str(Mode::M_AUTO)
        };

        this->traits.set_options(modes);
    }

    void ModeSelect::control(const std::string& state) {
        if (!this->has_option(state)) return;

        if (!this->parent->set_mode(str_to_mode(state)))
            this->publish_state(this->state);
    }
}
}
#include "auto_port_select.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.auto_port_select";

    void AutoPortSelect::setup() {
        parent->output_callback.add([this](int port) {
            esphome:optional<std::string> state = at(port-1);
            if (!state.has_value()) return;

            this->publish_state(state.value());
        }) ;

        this->traits.set_options(this->parent->get_ports());
    }

    void AutoPortSelect::control(const std::string& state) {
        if (!this->has_option(state)) return;

        if (!this->parent->set_mode(str_to_mode(state)))
            this->publish_state(this->state);
    }
}
}
#include "port_select.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.port_select";

    void PortSelect::setup() {
        this->parent->port_callback.add([this](int port) {
            esphome:optional<std::string> state = at(port-1);
            
            if (!state.has_value()) return;

            this->publish_state(state.value());
        }) ;

        this->traits.set_options(parent->get_ports());
    }

    void PortSelect::control(const std::string& state) {
        esphome::optional<size_t> index = this->index_of(state);

        if (!index.has_value()) return;

        if (!this->parent->set_port(index.value() + 1))
            this->publish_state(this->state);
    }
}
}
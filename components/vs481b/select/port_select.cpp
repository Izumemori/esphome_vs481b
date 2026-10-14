#include "port_select.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.port_select";

    void PortSelect::setup() {
        // Option pointers reference the parent's port names, which live for the whole program
        const std::vector<std::string> &ports = this->parent->get_ports();
        FixedVector<const char *> options;
        options.init(ports.size());
        for (const auto &port : ports)
            options.push_back(port.c_str());
        this->traits.set_options(options);

        this->parent->port_callback.add([this](int port) {
            if (port < 1 || !this->has_index(port - 1)) return;

            this->publish_state(static_cast<size_t>(port - 1));
        });
    }

    void PortSelect::control(size_t index) {
        // State is published by port_callback once the switch answers
        this->parent->set_port(index + 1);
    }
}
}

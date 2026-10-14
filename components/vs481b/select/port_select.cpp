#include "port_select.h"

namespace esphome{
namespace vs481b {
    static const char *const TAG = "vs481b.port_select";

    void PortSelect::setup() {
        // Option pointers reference the parent's port names, which live for the whole program
        const std::vector<std::string> &ports = this->parent_->get_ports();
        FixedVector<const char *> options;
        options.init(ports.size());
        for (const auto &port : ports)
            options.push_back(port.c_str());
        this->traits.set_options(options);

        this->parent_->port_callback.add([this](int port) {
            if (port < 1 || !this->has_index(port - 1)) return;

            this->publish_state(static_cast<size_t>(port - 1));
        });
    }

    void PortSelect::dump_config() {
        LOG_SELECT("", "VS481B Port Select", this);
    }

    void PortSelect::control(size_t index) {
        // State is published by port_callback once the switch answers
        this->parent_->set_port(index + 1);
    }
}
}

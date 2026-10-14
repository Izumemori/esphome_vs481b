#include "auto_port_select.h"

namespace esphome{
namespace vs481b {
    static const char *const TAG = "vs481b.auto_port_select";

    void AutoPortSelect::setup() {
        // Option pointers reference the parent's port names, which live for the whole program
        const std::vector<std::string> &ports = this->parent_->get_ports();
        FixedVector<const char *> options;
        options.init(ports.size());
        for (const auto &port : ports)
            options.push_back(port.c_str());
        this->traits.set_options(options);

        this->parent_->auto_port_callback.add([this](int port) {
            if (port < 1 || !this->has_index(port - 1)) return;

            this->publish_state(static_cast<size_t>(port - 1));
        });
    }

    void AutoPortSelect::dump_config() {
        LOG_SELECT("", "VS481B Auto Port Select", this);
    }

    void AutoPortSelect::control(size_t index) {
        // State is published by auto_port_callback once the switch answers
        this->parent_->set_auto_port(index + 1);
    }
}
}
